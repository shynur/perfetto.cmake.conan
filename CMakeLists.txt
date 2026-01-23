cmake_minimum_required(VERSION 3.23)

execute_process(
    COMMAND bash -c "[ -f VERSION ] && cat VERSION || git describe --tags --abbrev=0"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/perfetto
    OUTPUT_VARIABLE PERFETTO_GIT_TAG
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
string(SUBSTRING ${PERFETTO_GIT_TAG} 1 -1 PERFETTO_VERSION)
message(STATUS "Perfetto v${PERFETTO_VERSION}")

project(Perfetto
    VERSION ${PERFETTO_VERSION}
    LANGUAGES CXX
)

#---------------- config ---------------------
include(CheckSourceCompiles)

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

#---------------- dependencies ----------------
find_package(Threads)

#---------------- patch -----------------
add_custom_command(
    OUTPUT perfetto.patched.cc
    COMMAND patch -i ${PROJECT_SOURCE_DIR}/patches/perfetto.cc.patch -d ${PROJECT_SOURCE_DIR} -p0 -o ${CMAKE_CURRENT_BINARY_DIR}/perfetto.patched.cc
    DEPENDS patches/perfetto.cc.patch
    VERBATIM
)

#---------------- libperfetto ----------------
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
            FILE_SET shynur_polyfill_headers
            TYPE HEADERS
            BASE_DIRS polyfill
            FILES
                polyfill/charconv
                polyfill/shynur-polyfill.hpp
    )
endif()
target_link_libraries(perfetto
    PRIVATE
        Threads::Threads
)
target_compile_features(perfetto
    PUBLIC
        cxx_std_17
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
   TARGETS  perfetto
    EXPORT  PerfettoTargets
  FILE_SET  HEADERS
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
