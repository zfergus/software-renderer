# Detects whether this is a top-level project
get_directory_property(HAS_PARENT PARENT_DIRECTORY)
if(HAS_PARENT)
    set(SOFTWARE_RENDERER_TOPLEVEL_PROJECT OFF)
else()
    set(SOFTWARE_RENDERER_TOPLEVEL_PROJECT ON)
endif()

# Check required CMake version
set(REQUIRED_CMAKE_VERSION "3.14.0")
if(SOFTWARE_RENDERER_TOPLEVEL_PROJECT)
    cmake_minimum_required(VERSION ${REQUIRED_CMAKE_VERSION})
else()
    # Don't use cmake_minimum_required here to avoid implicitly overriding parent policies
    if(${CMAKE_VERSION} VERSION_LESS ${REQUIRED_CMAKE_VERSION})
        message(FATAL_ERROR "CMake required version to build Software Renderer is ${REQUIRED_CMAKE_VERSION}")
    endif()
endif()

# Include user-provided default options if available. We do that before the main
# `project()` so that we can define the C/C++ compilers from the option file.
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/SoftwareRendererOptions.cmake)
    message(STATUS "Using local options file: ${CMAKE_CURRENT_SOURCE_DIR}/SoftwareRendererOptions.cmake")
    include(${CMAKE_CURRENT_SOURCE_DIR}/SoftwareRendererOptions.cmake)
endif()

################################################################################

project(SoftwareRenderer
        DESCRIPTION "A basic headless software renderer."
        LANGUAGES CXX)

option(SOFTWARE_RENDERER_BUILD_EXAMPLE  "Build unit-tests"      ${SOFTWARE_RENDERER_TOPLEVEL_PROJECT})
option(SOFTWARE_RENDERER_WITH_SIMD      "Use SIMD instructions" OFF)

# Set default minimum C++ standard
if(RIGID_IPC_TOPLEVEL_PROJECT)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/software_renderer/")
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/recipes/")
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/find/")

# Color output
include(software_renderer_use_colors)

################################################################################
# Software Renderer Library
################################################################################

add_library(software_renderer
  src/camera.cpp
  src/raster.cpp
  src/render_mesh.cpp
  src/write_png.cpp
)
target_include_directories(software_renderer PUBLIC include)
add_library(software_renderer::software_renderer ALIAS software_renderer)

################################################################################
# Required Libraries
################################################################################

include(eigen)
include(libigl)
target_link_libraries(software_renderer PUBLIC igl::core)

include(stb)
target_link_libraries(software_renderer PUBLIC stb::stb)

include(onetbb)
target_link_libraries(software_renderer PUBLIC TBB::tbb)

include(json)
target_link_libraries(software_renderer PUBLIC nlohmann_json::nlohmann_json)

# Extra warnings
include(software_renderer_warnings)
target_link_libraries(software_renderer PRIVATE software_renderer::warnings)

################################################################################
# Compiler options
################################################################################

## SIMD support
if(SOFTWARE_RENDERER_WITH_SIMD)
  # Figure out SSE level support
  message(STATUS "Seaching for SSE...")
  find_package(SSE)
  # Apple broke AVX instructions with clang:
  # https://gitlab.com/libeigen/eigen/-/commit/71aa53dd6dfdc497324d9e87f59c4ba820191856
  if(NOT APPLE)
    # Figure out AVX level support
    message(STATUS "Searching for AVX...")
    find_package(AVX)
    # Figure out FMA level support
    message(STATUS "Searching for FMA...")
    find_package(FMA)
  else()
    message(WARNING "Disabling AVX support: clang compiler shipped with XCode 11.[012] generates broken assembly with -macosx-version-min=10.15 and AVX enabled.")
  endif()
  # Add SSE, AVX, and FMA flags to compiler flags
  string(REPLACE " " ";" SIMD_FLAGS "${SSE_FLAGS} ${AVX_FLAGS} ${FMA_FLAGS}")
  target_compile_options(software_renderer PUBLIC ${SIMD_FLAGS})
endif()

target_compile_features(software_renderer PUBLIC cxx_std_17)

################################################################################
# Example renderer
################################################################################

if(SOFTWARE_RENDERER_BUILD_EXAMPLE)
  add_subdirectory(example)
endif()
