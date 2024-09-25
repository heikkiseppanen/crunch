cmake_minimum_required(VERSION 3.25)

project( 
    Crunch
    VERSION 0.01
    LANGUAGES CXX C 
)

set(ENGINE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Engine)
set(LIB_DIR    ${CMAKE_CURRENT_SOURCE_DIR}/Lib   )

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE
    ${ENGINE_DIR}/Main.cpp

    ${ENGINE_DIR}/Core/Window.cpp
    ${ENGINE_DIR}/Core/Input.cpp

    #${ENGINE_DIR}/Graphics/Renderer.cpp
    ${ENGINE_DIR}/Graphics/Mesh.cpp
    ${ENGINE_DIR}/Graphics/SPIRVReflection.cpp

    ${ENGINE_DIR}/Graphics/Vulkan/Vulkan.cpp
    ${ENGINE_DIR}/Graphics/Vulkan/API.cpp
    ${ENGINE_DIR}/Graphics/Vulkan/Allocator.cpp
    ${ENGINE_DIR}/Graphics/Vulkan/Extensions.cpp
    ${ENGINE_DIR}/Graphics/Vulkan/Buffer.cpp
    ${ENGINE_DIR}/Graphics/Vulkan/Queue.cpp
    ${ENGINE_DIR}/Graphics/Vulkan/CommandBuffer.cpp
    ${ENGINE_DIR}/Graphics/Vulkan/ShaderModule.cpp
    ${ENGINE_DIR}/Graphics/Vulkan/Shader.cpp
    ${ENGINE_DIR}/Graphics/Vulkan/Texture.cpp

    ${ENGINE_DIR}/Crunch/Filesystem.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE ${ENGINE_DIR})

add_subdirectory(${LIB_DIR})

target_compile_options(
    ${PROJECT_NAME}
    PUBLIC
        $<$<CONFIG:Debug>: -g > #-fsanitize=address -fno-omit-frame-pointer >
        -std=c++23
        -Wall -Werror -Wextra -Wno-missing-field-initializers
)

target_link_options(
    ${PROJECT_NAME}
    PUBLIC
        $<$<CONFIG:Debug>: -g > #-fsanitize=address -fno-omit-frame-pointer >
)
