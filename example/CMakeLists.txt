################################################################################
# Example OBJ sequence renderer
################################################################################

add_executable(render_objs
  render_objs.cpp
)

################################################################################
# Required Libraries
################################################################################

# Extra warnings
include(software_renderer_warnings)
target_link_libraries(software_renderer PRIVATE software_renderer::warnings)

target_link_libraries(render_objs PUBLIC software_renderer::software_renderer)

include(spdlog)
target_link_libraries(render_objs PUBLIC spdlog::spdlog)

include(cli11)
target_link_libraries(render_objs PUBLIC CLI11::CLI11)

include(filesystem)
target_link_libraries(render_objs PUBLIC ghc::filesystem)

################################################################################
# Compiler options
################################################################################

target_compile_features(render_objs PUBLIC cxx_std_14)
