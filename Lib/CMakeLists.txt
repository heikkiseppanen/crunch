cmake_minimum_required(VERSION 3.25)

#===========================================================#
# Third party dependencies                                  #
#===========================================================#

find_package(Vulkan)

IF (NOT Vulkan_FOUND)
    find_library(Vulkan_LIBRARY NAMES vulkan-1 vulkan PATHS ${CMAKE_CURRENT_LIST_DIRECTORY}/Vulkan)
    IF (Vulkan_LIBRARY)
        message("Falling back to included static Vulkan library")
        set(Vulkan_INCLUDE_DIRS Vulkan/include)
        set(Vulkan_FOUND ON)
    ELSE ()
        message(FATAL_ERROR "No Vulkan library could be found!")
    ENDIF()
ENDIF()

find_library(ktx_LIBRARY NAMES ktx PATHS ${CMAKE_CURRENT_LIST_DIRECTORY}/KTX)

IF (ktx_LIBRARY)
    set(ktx_FOUND ON)
ELSE()
    message(FATAL_ERROR "No KTX library could be found!")
ENDIF()

set(GLFW_BUILD_TESTS    OFF CACHE BOOL "Lib only" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Lib only" FORCE)
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "Lib only" FORCE)
set(GLFW_INSTALL        OFF CACHE BOOL "Lib only" FORCE)
add_subdirectory(GLFW)

add_subdirectory(GLM )

target_sources(${PROJECT_NAME} PRIVATE
    SPIRV-Reflect/spirv_reflect.cpp
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        glfw
        ${Vulkan_LIBRARIES}
        ${ktx_LIBRARY}
    INTERFACE
        glm
)

message("${Vulkan_INCLUDE_DIRS}")

target_include_directories(${PROJECT_NAME} PRIVATE
    GLFW/include
    GLM
    KTX/include
    VulkanMemoryAllocator
    SPIRV-Reflect
    ${Vulkan_INCLUDE_DIRS}
)



