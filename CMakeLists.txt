cmake_minimum_required(VERSION 3.0)
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
	external/Soil/src/
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
	
    shaders/TerrainGenerator.vertex.glsl
    shaders/TerrainGenerator.fragment.glsl
    shaders/AtmosphericScattering.vertex.glsl
    shaders/AtmosphericScattering.fragment.glsl
)

target_link_libraries(ProceduralPlanets
    ${OPENGL_LIBRARY}
    glfw
	GLEW_190
    SOIL
)

set_property(TARGET ProceduralPlanets PROPERTY CXX_STANDARD 14)
target_compile_options(ProceduralPlanets PRIVATE "-Wall" "-Wextra" "-Wpedantic")
