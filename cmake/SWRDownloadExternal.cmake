include(DownloadProject)

# With CMake 3.8 and above, we can hide warnings about git being in a
# detached head by passing an extra GIT_CONFIG option
if(NOT (${CMAKE_VERSION} VERSION_LESS "3.8.0"))
  set(SWR_EXTRA_OPTIONS "GIT_CONFIG advice.detachedHead=false")
else()
  set(SWR_EXTRA_OPTIONS "")
endif()

function(swr_download_project name)
  download_project(
    PROJ         ${name}
    SOURCE_DIR   ${SWR_EXTERNAL}/${name}
    DOWNLOAD_DIR ${SWR_EXTERNAL}/.cache/${name}
    QUIET
    ${SWR_EXTRA_OPTIONS}
    ${ARGN}
  )
endfunction()

################################################################################

function(swr_download_libigl)
  swr_download_project(libigl
    GIT_REPOSITORY https://github.com/libigl/libigl.git
    GIT_TAG        v2.3.0
  )
endfunction()

function(swr_download_json)
  swr_download_project(json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.9.1
  )
endfunction()

function(swr_download_tbb)
  swr_download_project(tbb
  GIT_REPOSITORY https://github.com/wjakob/tbb.git
  GIT_TAG        141b0e310e1fb552bdca887542c9c1a8544d6503
  )
endfunction()
