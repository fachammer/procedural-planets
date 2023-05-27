cmake_minimum_required(VERSION 3.26)
project(procedural-planets)

find_package(OpenGL REQUIRED)

if(CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR)
    message( FATAL_ERROR "Please select another Build Directory! (e.g. build/)" )
endif()

add_subdirectory(external)

include_directories(
	external/glfw-3.3.8/include/GLFW/
	external/glm-0.9.9.8/
	external/glew-1.9.0/include/
	.
)

add_definitions(
	-DTW_STATIC
	-DTW_NO_LIB_PRAGMA
	-DTW_NO_DIRECT3D
	-DGLEW_STATIC
	-D_CRT_SECURE_NO_WARNINGS
)

add_executable(ProceduralPlanets
	src/ProceduralPlanets.cpp
	src/GlResources.hpp
)

target_link_libraries(ProceduralPlanets
    ${OPENGL_LIBRARY}
    glfw
	GLEW_190
)

set_property(TARGET ProceduralPlanets PROPERTY CXX_STANDARD 20)

add_custom_target(copy_assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/assets
)
add_dependencies(ProceduralPlanets copy_assets)

