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
	src/SphereGenerator.hpp
	src/SphereGenerator.cpp
	src/Shader.cpp
	src/Shader.hpp
	src/Texture.cpp
	src/Texture.hpp
	src/Mesh.hpp
	src/Mesh.cpp
	src/GLError.h
	src/GLError.cpp
	
    shaders/TerrainGenerator.vertexshader
    shaders/TerrainGenerator.geometryshader
    shaders/TerrainGenerator.fragmentshader
    shaders/AtmosphericScattering.vertexshader
    shaders/AtmosphericScattering.fragmentshader
)

target_link_libraries(ProceduralPlanets
    ${OPENGL_LIBRARY}
    glfw
	GLEW_190
    SOIL
)

set_property(TARGET ProceduralPlanets PROPERTY CXX_STANDARD 11)
