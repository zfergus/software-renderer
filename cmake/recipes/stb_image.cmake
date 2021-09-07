if(TARGET igl::stbimage)
    return()
endif()

message(STATUS "Third-party: creating target 'igl::stbimage'")

include(eigen)
include(libigl)

if(NOT TARGET stb_image)
  # Download this myself for the software renderer
  igl_download_stb()
  add_subdirectory(${LIBIGL_EXTERNAL}/stb stb_image)
  add_library(igl::stbimage ALIAS igl_stb_image)
endif()
