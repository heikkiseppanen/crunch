cmake_minimum_required(VERSION 3.25)

#===========================================================#
# Third party dependencies                                  #
#===========================================================#

find_package( Vulkan )

IF (NOT Vulkan_FOUND)
	find_library(Vulkan_LIBRARY NAMES vulkan-1 vulkan PATHS ${CMAKE_SOURCE_DIR}/Lib/Vulkan)
	IF (Vulkan_LIBRARY)
		set(Vulkan_FOUND ON)
	ENDIF()
ENDIF()

IF (NOT Vulkan_FOUND)
	message(FATAL_ERROR "No Vulkan library could be found!")
ENDIF()

find_library(ktx_LIBRARY NAMES ktx PATHS ${CMAKE_SOURCE_DIR}/Lib/KTX)

IF (ktx_LIBRARY)
	set(ktx_FOUND ON)
ELSE()
	message(FATAL_ERROR "No KTX library could be found!")
ENDIF()

set( GLFW_BUILD_TESTS OFF CACHE BOOL "Lib only" FORCE)
set( GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Lib only" FORCE)
set( GLFW_BUILD_DOCS OFF CACHE BOOL "Lib only" FORCE)
set( GLFW_INSTALL OFF CACHE BOOL "Lib only" FORCE)

add_subdirectory( GLFW )

add_subdirectory( GLM )

target_link_libraries(${PROJECT_NAME}
		PUBLIC
			vulkan
			glfw
			ktx
		INTERFACE
			glm
)

target_include_directories(${PROJECT_NAME} PRIVATE
	"${LIB_DIR}/GLFW/include"
	"${LIB_DIR}/GLM"
	"${LIB_DIR}/KTX/include"
	"${LIB_DIR}/Vulkan/include"
	"${LIB_DIR}/VulkanMemoryAllocator"
)



