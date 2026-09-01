cmake_minimum_required(VERSION 3.23)

execute_process(
    COMMAND bash -c "[ -f VERSION ] && cat VERSION || git describe --tags --abbrev=0"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/perfetto
    OUTPUT_VARIABLE PERFETTO_GIT_TAG
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
# 提前校验 tag: 空值 (shallow clone 且无 VERSION 文件, git describe 失败) 若直接
# 进入下面的 string(SUBSTRING) 会以晦涩的内部错误中断 configure; 在此给出可操作的
# 友好诊断.  下方下载块因而无需再重复校验.
if(NOT PERFETTO_GIT_TAG MATCHES "^v[0-9]")
    message(FATAL_ERROR
        "无法确定 Perfetto 版本 tag (PERFETTO_GIT_TAG='${PERFETTO_GIT_TAG}'). "
        "请在 submodule 内执行 `git fetch --tags`.")
endif()
string(SUBSTRING "${PERFETTO_GIT_TAG}" 1 -1 PERFETTO_VERSION)
message(STATUS "Perfetto v${PERFETTO_VERSION}")

#---------------- fetch amalgamated SDK (perfetto v54+) ----------------
# v53 及更早: sdk/perfetto.{cc,h} 提交在 submodule git 树里, 此块跳过.
# v54 起: 上游不再把 SDK 提交进仓库, 改为以 release 资产 perfetto-cpp-sdk-src.zip
#         单独发布, 需在配置期下载并放回 perfetto/sdk/ 老位置.
set(PERFETTO_SDK_DIR ${CMAKE_CURRENT_SOURCE_DIR}/perfetto/sdk)
set(PERFETTO_SDK_URL
    "https://github.com/google/perfetto/releases/download/${PERFETTO_GIT_TAG}/perfetto-cpp-sdk-src.zip"
    CACHE STRING "URL of the Perfetto amalgamated C++ SDK source zip")

set(_perfetto_need_sdk TRUE)
if(EXISTS ${PERFETTO_SDK_DIR}/perfetto.cc
   AND EXISTS ${PERFETTO_SDK_DIR}/perfetto.h
   AND EXISTS ${PERFETTO_SDK_DIR}/.sdk-version)
    # 下载得来的 SDK: 用版本戳记判断是否与当前 tag 匹配, 不匹配则重下,
    # 避免 submodule 升级 tag 后旧 SDK 残留造成版本错配.
    file(READ ${PERFETTO_SDK_DIR}/.sdk-version _perfetto_cached_ver)
    string(STRIP "${_perfetto_cached_ver}" _perfetto_cached_ver)
    if(_perfetto_cached_ver STREQUAL PERFETTO_GIT_TAG)
        set(_perfetto_need_sdk FALSE)
    endif()
elseif(EXISTS ${PERFETTO_SDK_DIR}/perfetto.cc AND EXISTS ${PERFETTO_SDK_DIR}/perfetto.h)
    # v53 及更早: 文件由 submodule 自带, 无 .sdk-version 戳记, 视为就位.
    set(_perfetto_need_sdk FALSE)
endif()

if(_perfetto_need_sdk)
    message(STATUS "Fetching Perfetto SDK ${PERFETTO_GIT_TAG} from ${PERFETTO_SDK_URL}")
    set(_perfetto_zip ${CMAKE_CURRENT_BINARY_DIR}/perfetto-cpp-sdk-src.zip)
    file(DOWNLOAD ${PERFETTO_SDK_URL} ${_perfetto_zip} TLS_VERIFY ON STATUS _perfetto_dl_st)
    list(GET _perfetto_dl_st 0 _perfetto_dl_code)
    if(NOT _perfetto_dl_code EQUAL 0)
        list(GET _perfetto_dl_st 1 _perfetto_dl_msg)
        file(REMOVE ${_perfetto_zip})
        message(FATAL_ERROR "下载 Perfetto SDK 失败 (${PERFETTO_SDK_URL}): ${_perfetto_dl_msg}")
    endif()
    set(_perfetto_extract ${CMAKE_CURRENT_BINARY_DIR}/perfetto-sdk-extract)
    file(REMOVE_RECURSE ${_perfetto_extract})
    file(ARCHIVE_EXTRACT INPUT ${_perfetto_zip} DESTINATION ${_perfetto_extract})
    file(MAKE_DIRECTORY ${PERFETTO_SDK_DIR})
    # 先清除旧的 SDK 与戳记: 万一 COPY 与写戳记之间 configure 被中断 (Ctrl-C/OOM),
    # 留下的将是"无戳记"的残缺状态, 下次 configure 会重新下载, 而不会被下面的
    # 向后兼容分支误判成 v53 自带文件而跳过.
    file(REMOVE
        ${PERFETTO_SDK_DIR}/.sdk-version
        ${PERFETTO_SDK_DIR}/perfetto.cc
        ${PERFETTO_SDK_DIR}/perfetto.h)
    file(COPY
        ${_perfetto_extract}/perfetto.h
        ${_perfetto_extract}/perfetto.cc
        DESTINATION ${PERFETTO_SDK_DIR})
    file(WRITE ${PERFETTO_SDK_DIR}/.sdk-version "${PERFETTO_GIT_TAG}\n")
endif()

project(Perfetto
    VERSION ${PERFETTO_VERSION}
    LANGUAGES CXX
)

#---------------- config ---------------------
include(CheckSourceCompiles)

# perfetto 以 C++17 编译 (见下方 cxx_std_17).  check_source_compiles 默认使用
# 编译器的默认标准 (常为 C++14), 这会让 std::launder / std::from_chars 等被
# `#if __cplusplus >= 201703L` 守护的特性在检测时不可见 -> 假阴性 -> 误注入
# polyfill, 进而与真实 C++17 编译时系统已提供的声明冲突.  故让检测也用 C++17.
# (cmake_minimum_required(3.23) 使 CMP0067=NEW, try_compile 会沿用下列变量.)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

check_source_compiles(CXX
    [[
        #include <charconv>
        #include <string>

        int main() {
            const auto s = std::string{"0x111"};
            int result;
            std::from_chars(s.c_str(), &*s.cend(), result, 16);
        }
    ]]
    HAS_CXX_FROM_CHARS
)

# perfetto v54+ 的 protovm 模块用到 std::launder (C++17, <new>), 但旧标准库
# (如 Clang 6 配套的旧 libstdc++) 不提供它.  独立检测, 缺失时由 polyfill/new 补齐.
check_source_compiles(CXX
    [[
        #include <new>

        int main() {
            int x = 0;
            return std::launder(&x) == nullptr;
        }
    ]]
    HAS_CXX_LAUNDER
)

#---------------- dependencies ----------------
find_package(Threads)

#---------------- patch -----------------
# perfetto.cc.patch — 基线补丁 (weak tr_expand_tag / SequenceState 拷贝语义等),
# 用 -o 从原始 SDK 生成 perfetto.patched.cc; 每次都从源码重新生成, 可安全重复执行.
add_custom_command(
    OUTPUT perfetto.patched.cc
    COMMAND patch -i ${PROJECT_SOURCE_DIR}/patches/perfetto.cc.patch -d ${PROJECT_SOURCE_DIR} -p0 -o ${CMAKE_CURRENT_BINARY_DIR}/perfetto.patched.cc
    DEPENDS patches/perfetto.cc.patch ${PERFETTO_SDK_DIR}/perfetto.cc
    VERBATIM
)

#---------------- libperfetto ----------------
include(CheckIPOSupported)
check_ipo_supported(RESULT Perfetto_IPO_supported OUTPUT output)

add_library(perfetto)
add_library(Perfetto::perfetto ALIAS perfetto)
target_sources(perfetto
    PRIVATE
        perfetto.patched.cc
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS perfetto/sdk
        FILES
            perfetto/sdk/perfetto.h
)
if(NOT HAS_CXX_FROM_CHARS)
    target_sources(perfetto
        PRIVATE
            FILE_SET shynur_polyfill_charconv
            TYPE HEADERS
            BASE_DIRS polyfill
            FILES
                polyfill/charconv
                polyfill/shynur-polyfill.hpp
    )
endif()
if(NOT HAS_CXX_LAUNDER)
    target_sources(perfetto
        PRIVATE
            FILE_SET shynur_polyfill_new
            TYPE HEADERS
            BASE_DIRS polyfill
            FILES
                polyfill/new
    )
endif()
target_link_libraries(perfetto
    PUBLIC
        Threads::Threads
)
target_compile_features(perfetto
    PUBLIC
        cxx_std_17
)
if(Perfetto_IPO_supported)
    #set_target_properties(perfetto
    #    PROPERTIES
    #        INTERPROCEDURAL_OPTIMIZATION TRUE
    #)
endif()
set_target_properties(perfetto
    PROPERTIES
        POSITION_INDEPENDENT_CODE ON
)
if(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "GNU")
    target_compile_options(perfetto
        PRIVATE
            -mstackrealign -mno-avx -mno-avx2 -mno-avx512f -mno-fma -mno-bmi -mno-bmi2
    )  # RBK requirement
endif()

add_library(entire_include INTERFACE)
add_library(Perfetto::entire_include ALIAS entire_include)
file(GLOB_RECURSE
    Perfetto_include_headers
    ${CMAKE_CURRENT_SOURCE_DIR}/perfetto/include/perfetto/*
)
target_sources(entire_include
    INTERFACE
        FILE_SET HEADERS
        BASE_DIRS
            perfetto/include
        FILES
            ${Perfetto_include_headers}
)
target_link_libraries(entire_include
    INTERFACE
        perfetto
)
#---------------- platform specific ----------------
if(WIN32)
    target_compile_options(perfetto PRIVATE /bigobj)
    target_compile_definitions(perfetto
        PUBLIC
            -DWIN32_LEAN_AND_MEAN
            -DNOMINMAX
    )
    target_link_libraries(perfetto PRIVATE ws2_32)
endif()

if(MSVC)
    target_compile_options(perfetto INTERFACE /permissive-)
endif()
#--------------- install -------------------
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(
    TARGETS
        perfetto
        entire_include  # RBK 用到了这玩意, 我也很无奈.
    EXPORT
        PerfettoTargets
    FILE_SET
        HEADERS
)

install(
       EXPORT  PerfettoTargets
  DESTINATION  ${CMAKE_INSTALL_LIBDIR}/cmake/Perfetto
    NAMESPACE  Perfetto::
)

install(
    FILES
        cmake/PerfettoConfig.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Perfetto
)


write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/PerfettoConfigVersion.cmake
    COMPATIBILITY SameMajorVersion
)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/PerfettoConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Perfetto
)
