cmake_minimum_required(VERSION 3.20)

project( 
	Crunch
	VERSION 0.01
	LANGUAGES CXX C 
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

set(ENGINE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Engine)
set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Lib)

set(SOURCE
	"${ENGINE_DIR}/Main.cpp"
	"${ENGINE_DIR}/Window/Window.cpp"
	"${ENGINE_DIR}/Graphics/Vulkan/API.cpp"
	"${ENGINE_DIR}/Graphics/Vulkan/Allocator.cpp"
	"${ENGINE_DIR}/Graphics/Vulkan/Extension.cpp"
	"${ENGINE_DIR}/Graphics/Mesh.cpp"
	"${ENGINE_DIR}/Shared/Filesystem.cpp"
)

add_executable(${PROJECT_NAME} ${SOURCE})

target_include_directories(${PROJECT_NAME} PRIVATE
	"${ENGINE_DIR}"
	"${LIB_DIR}/GLFW/include"
	"${LIB_DIR}/VulkanMemoryAllocator"
)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Lib")

target_compile_options(${PROJECT_NAME} PUBLIC -Wall -Werror -Wextra)
