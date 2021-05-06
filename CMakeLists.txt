cmake_minimum_required(VERSION 3.8)
project(swr)

################################################################################
# Project Options
################################################################################

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(SWR_TOPLEVEL_PROJECT ON)
else()
    set(SWR_TOPLEVEL_PROJECT OFF)
endif()

if(SWR_TOPLEVEL_PROJECT)
  option(SWR_BUILD_EXAMPLE  "Build unit-tests"  ON)
else()
  option(SWR_BUILD_EXAMPLE  "Build unit-tests"  OFF)
endif()

################################################################################
# Configuration
################################################################################

set(SWR_EXTERNAL "${CMAKE_CURRENT_SOURCE_DIR}/external")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(Warnings)
include(UseColors)
include(SWRDependencies)

################################################################################
# Software Renderer Library
################################################################################

add_library(${PROJECT_NAME}
  src/camera.cpp
  src/raster.cpp
  src/render_mesh.cpp
  src/write_png.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC include)

target_link_libraries(${PROJECT_NAME} PUBLIC igl::core igl::stbimage)
target_link_libraries(${PROJECT_NAME} PUBLIC TBB::tbb)
target_link_libraries(${PROJECT_NAME} PUBLIC nlohmann::json)

################################################################################
# Example renderer
################################################################################

if(SWR_BUILD_EXAMPLE)
  add_subdirectory(example)
endif()

################################################################################
# Required Libraries
################################################################################

# Extra warnings
target_link_libraries(${PROJECT_NAME} PRIVATE warnings::all)

# Eigen
target_link_libraries(${PROJECT_NAME} PUBLIC Eigen3::Eigen)

# TBB
target_link_libraries(${PROJECT_NAME} PUBLIC TBB::tbb)

# JSON Parser
target_link_libraries(${PROJECT_NAME} PUBLIC nlohmann::json)

################################################################################
# Compiler options
################################################################################

## SIMD support
if(SWR_WITH_SIMD)
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
  target_compile_options(${PROJECT_NAME} PUBLIC ${SIMD_FLAGS})
endif()

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_11)
