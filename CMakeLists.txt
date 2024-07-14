cmake_minimum_required(VERSION 3.25)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

project( 
    Crunch
    VERSION 0.01
    LANGUAGES CXX C 
)

set(ENGINE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Engine)
set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Lib)

set(SOURCE
    "${ENGINE_DIR}/Main.cpp"

    "${ENGINE_DIR}/Core/Window.cpp"
    "${ENGINE_DIR}/Core/Input.cpp"

    "${ENGINE_DIR}/Graphics/API.cpp"
    "${ENGINE_DIR}/Graphics/Mesh.cpp"

    "${ENGINE_DIR}/Graphics/Vulkan/Vulkan.cpp"
    "${ENGINE_DIR}/Graphics/Vulkan/Allocator.cpp"
    "${ENGINE_DIR}/Graphics/Vulkan/Driver.cpp"
    "${ENGINE_DIR}/Graphics/Vulkan/Buffer.cpp"
    "${ENGINE_DIR}/Graphics/Vulkan/Extensions.cpp"

    "${ENGINE_DIR}/Shared/Filesystem.cpp"
)

add_executable(${PROJECT_NAME} ${SOURCE})

target_include_directories(${PROJECT_NAME} PRIVATE
    "${ENGINE_DIR}"
)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Lib")

target_compile_options(
    ${PROJECT_NAME}
    PUBLIC
        $<$<CONFIG:Debug>: -g > #-fsanitize=address -fno-omit-frame-pointer >
        -std=c++20
        -Wall -Werror -Wextra -Wno-missing-field-initializers
)

target_link_options(
    ${PROJECT_NAME}
    PUBLIC
        $<$<CONFIG:Debug>: -g > #-fsanitize=address -fno-omit-frame-pointer >
)
