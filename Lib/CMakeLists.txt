cmake_minimum_required(VERSION 3.20)

#===========================================================#
# Third party dependencies                                  #
#===========================================================#

find_package( Vulkan REQUIRED )

set( GLFW_BUILD_TESTS OFF CACHE BOOL "Lib only" FORCE)
set( GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Lib only" FORCE)
set( GLFW_BUILD_DOCS OFF CACHE BOOL "Lib only" FORCE)
set( GLFW_INSTALL OFF CACHE BOOL "Lib only" FORCE)

add_subdirectory(GLFW)

target_link_libraries(${PROJECT_NAME} vulkan glfw)



