# Prepare dependencies
#
# For each third-party library, if the appropriate target doesn't exist yet,
# download it via external project, and add_subdirectory to build it alongside
# this project.


# Download and update 3rd_party libraries
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})
list(REMOVE_DUPLICATES CMAKE_MODULE_PATH)
include(SWRDownloadExternal)

################################################################################
# Required libraries
################################################################################

# libigl
if(NOT TARGET igl::core)
  swr_download_libigl()
  # Import libigl targets
  list(APPEND CMAKE_MODULE_PATH "${SWR_EXTERNAL}/libigl/cmake")
  include(libigl)

  if(NOT TARGET stb_image)
    # Download this myself for the software renderer
    igl_download_stb()
    add_subdirectory(${LIBIGL_EXTERNAL}/stb stb_image)
    add_library(igl::stbimage ALIAS igl_stb_image)
  endif()
endif()

# TBB
if(NOT TARGET TBB::tbb)
  swr_download_tbb()
  set(TBB_BUILD_STATIC ON CACHE BOOL " " FORCE)
  set(TBB_BUILD_SHARED OFF CACHE BOOL " " FORCE)
  set(TBB_BUILD_TBBMALLOC OFF CACHE BOOL " " FORCE)
  set(TBB_BUILD_TBBMALLOC_PROXY OFF CACHE BOOL " " FORCE)
  set(TBB_BUILD_TESTS OFF CACHE BOOL " " FORCE)
  add_subdirectory(${SWR_EXTERNAL}/tbb EXCLUDE_FROM_ALL)
  add_library(TBB::tbb ALIAS tbb_static)
endif()

# json
if(NOT TARGET nlohmann::json)
  swr_download_json()
  option(JSON_BuildTests "" OFF)
  option(JSON_MultipleHeaders "" ON)
  add_subdirectory(${SWR_EXTERNAL}/json json)
  add_library(nlohmann::json ALIAS nlohmann_json)
endif()
