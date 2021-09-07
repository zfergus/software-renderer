if(TARGET igl::stbimage)
    return()
endif()

message(STATUS "Third-party: creating target 'igl::stbimage'")

include(eigen)
include(libigl)

# This may have been downloaded by libigl depending on the OpenGL options
if(NOT TARGET igl_stb_image)
    igl_download_stb()
    add_subdirectory(${LIBIGL_EXTERNAL}/stb stb_image)
endif()

add_library(igl::stbimage ALIAS igl_stb_image)
