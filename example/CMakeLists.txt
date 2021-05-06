cmake_minimum_required(VERSION 3.8)

################################################################################
# Example OBJ sequence renderer
################################################################################

add_executable(render_objs
  render_objs.cpp
)

target_link_libraries(render_objs PUBLIC ${PROJECT_NAME})

################################################################################
# Required Libraries
################################################################################

function(swr_download_spdlog)
  swr_download_project(spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG        v1.8.5
  )
endfunction()

function(swr_download_cli11)
  swr_download_project(cli11
    GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
    GIT_TAG        8ecce8fd2c49f64c80e5757cb12d2fd1fa62f242
  )
endfunction()

## filesystem library for C++11 and C++14
function(swr_download_filesystem)
  swr_download_project(filesystem
    GIT_REPOSITORY https://github.com/gulrak/filesystem.git
    GIT_TAG        v1.5.4
  )
endfunction()

################################################################################

# spdlog
if(NOT TARGET spdlog::spdlog)
  swr_download_spdlog()
  add_library(spdlog INTERFACE)
  add_library(spdlog::spdlog ALIAS spdlog)
  target_include_directories(spdlog SYSTEM INTERFACE ${SWR_EXTERNAL}/spdlog/include)
endif()

# CLI11
if(NOT TARGET CLI11::CLI11)
  swr_download_cli11()
  add_subdirectory(${SWR_EXTERNAL}/cli11 ${CMAKE_CURRENT_BINARY_DIR}/cli11)
endif()

# GHC Filesystem
if(NOT TARGET ghc::filesystem)
  swr_download_filesystem()
  add_subdirectory(${SWR_EXTERNAL}/filesystem ${CMAKE_CURRENT_BINARY_DIR}/filesystem)
  add_library(ghc::filesystem ALIAS ghc_filesystem)
endif()

################################################################################

# Extra warnings
target_link_libraries(render_objs PRIVATE warnings::all)

# CLI11
target_link_libraries(render_objs PUBLIC CLI11::CLI11)

# Logger
target_link_libraries(render_objs PUBLIC spdlog::spdlog)

# GHC Filesystem
target_link_libraries(render_objs PUBLIC ghc::filesystem)

################################################################################
# Compiler options
################################################################################

target_compile_features(render_objs PUBLIC cxx_std_14)
