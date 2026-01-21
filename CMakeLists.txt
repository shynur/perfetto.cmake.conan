cmake_minimum_required(VERSION 3.23)

execute_process(
    COMMAND ls -A
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
execute_process(
    COMMAND git describe --tags --abbrev=0
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


find_package(Threads)


add_library(perfetto)
target_sources(perfetto
    PRIVATE
        perfetto/sdk/perfetto.cc
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS perfetto/sdk
        FILES
            perfetto/sdk/perfetto.h
)
target_link_libraries(perfetto
    PRIVATE
        ${CMAKE_THREAD_LIBS_INIT}
)


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
