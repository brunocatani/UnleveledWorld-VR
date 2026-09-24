cmake_minimum_required(VERSION 3.21)

set(COMMON_LIB_F4VR_PATH ""
    CACHE PATH "Path to CommonLibF4VR")
if (NOT EXISTS "${COMMON_LIB_F4VR_PATH}/CMakeLists.txt")
    message(FATAL_ERROR "Set COMMON_LIB_F4VR_PATH to the CommonLibF4VR/CommonLibF4 directory in CMakeUserPresets.json.")
endif ()

# ---- vcpkg setup ----

if (DEFINED ENV{VCPKG_INSTALLATION_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_INSTALLATION_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif ()
set(VCPKG_TARGET_TRIPLET "x64-windows-static" CACHE STRING "")

# ---- Project ----

project(
    GLXRM_UnleveledWorld
    VERSION 1.0.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE STRING "")
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION OFF)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/Version.h
    @ONLY
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/version.rc
    @ONLY
)

# ---- In-source build guard ----

if (PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
    message(FATAL_ERROR "In-source builds not allowed.")
endif ()

# ---- CommonLibF4VR ----

set(ENABLE_FALLOUT_F4 OFF CACHE BOOL "" FORCE)
set(ENABLE_FALLOUT_NG OFF CACHE BOOL "" FORCE)
set(ENABLE_FALLOUT_VR ON CACHE BOOL "" FORCE)
set(F4SE_SUPPORT_XBYAK ON CACHE BOOL "" FORCE)
add_subdirectory(${COMMON_LIB_F4VR_PATH} "CommonLibF4" EXCLUDE_FROM_ALL)

# ---- Dependencies ----

find_package(fmt REQUIRED CONFIG)
find_package(spdlog REQUIRED CONFIG)

# ---- Source files ----

include(cmake/sourcelist.cmake)

source_group(
    TREE ${CMAKE_CURRENT_SOURCE_DIR}
    FILES ${SOURCES}
)

source_group(
    TREE ${CMAKE_CURRENT_BINARY_DIR}
    FILES ${CMAKE_CURRENT_BINARY_DIR}/include/Version.h
)

# ---- Create DLL ----

add_library(
    ${PROJECT_NAME}
    SHARED
    ${SOURCES}
    ${CMAKE_CURRENT_BINARY_DIR}/version.rc
)

target_compile_features(
    ${PROJECT_NAME}
    PRIVATE
        cxx_std_23
)

target_include_directories(
    ${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE
        CommonLibF4::CommonLibF4
        fmt::fmt
        spdlog::spdlog
        shell32 ole32 uuid
)

target_precompile_headers(
    ${PROJECT_NAME}
    PRIVATE
        src/PCH.h
)

# Configuration also runs in standalone filesystem tests without engine headers.
set_source_files_properties(src/Configuration.cpp PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
enable_testing()
add_executable(configuration_tests tests/ConfigurationTests.cpp src/Configuration.cpp)
target_compile_features(configuration_tests PRIVATE cxx_std_23)
target_compile_options(configuration_tests PRIVATE /utf-8)
target_include_directories(configuration_tests PRIVATE src)
add_test(NAME configuration_tests COMMAND configuration_tests)

if (MSVC)
    target_compile_options(
        ${PROJECT_NAME}
        PRIVATE
            /sdl /utf-8 /Zi
            /permissive- /Zc:preprocessor
            /W4
            /wd4715
            "$<$<CONFIG:RELEASE>:/Zc:inline;/JMC-;/Ob3>"
    )

    target_link_options(
        ${PROJECT_NAME}
        PRIVATE
            "$<$<CONFIG:DEBUG>:/INCREMENTAL;/OPT:NOREF;/OPT:NOICF>"
            "$<$<CONFIG:RELEASE>:/INCREMENTAL:NO;/OPT:REF;/OPT:ICF;/DEBUG:FULL>"
    )
endif ()

set(UNLEVELED_WORLD_DEPLOY_DIR "" CACHE PATH "Local MO2 mod directory for DLL/PDB deployment")
if (UNLEVELED_WORLD_DEPLOY_DIR)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${UNLEVELED_WORLD_DEPLOY_DIR}/F4SE/Plugins"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:${PROJECT_NAME}>"
            "$<TARGET_PDB_FILE:${PROJECT_NAME}>"
            "${UNLEVELED_WORLD_DEPLOY_DIR}/F4SE/Plugins"
        COMMENT "Deploying Unleveled World DLL/PDB to ${UNLEVELED_WORLD_DEPLOY_DIR}"
        VERBATIM
    )
endif ()
