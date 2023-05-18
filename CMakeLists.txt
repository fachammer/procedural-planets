# CMake entry point
cmake_minimum_required (VERSION 2.6)
project (Tutorials)

find_package(OpenGL REQUIRED)

if( CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR )
    message( FATAL_ERROR "Please select another Build Directory ! (and give it a clever name, like bin_Visual2012_64bits/)" )
endif()
if( CMAKE_SOURCE_DIR MATCHES " " )
	message( "Your Source Directory contains spaces. If you experience problems when compiling, this can be the cause." )
endif()
if( CMAKE_BINARY_DIR MATCHES " " )
	message( "Your Build Directory contains spaces. If you experience problems when compiling, this can be the cause." )
endif()

# Compile external dependencies
add_subdirectory (external)

# On Visual 2005 and above, this module can set the debug working directory
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/external/rpavlik-cmake-modules-1c73e35")
include(CreateLaunchers)
include(MSVCMultipleProcessCompile) # /MP

if(INCLUDE_DISTRIB)
	add_subdirectory(distrib)
endif(INCLUDE_DISTRIB)

include_directories(
	external/glfw-3.3.8/include/GLFW/
	external/glm-0.9.4.0/
	external/glew-1.9.0/include/
	external/Soil/src/
	#external/bullet-2.81-rev2613/src/
	.
)

set(ALL_LIBS
	${OPENGL_LIBRARY}
	glfw
	GLEW_190
)

add_definitions(
	-DTW_STATIC
	-DTW_NO_LIB_PRAGMA
	-DTW_NO_DIRECT3D
	-DGLEW_STATIC
	-D_CRT_SECURE_NO_WARNINGS
)

add_executable(PlanetGenerator
	src/PlanetGenerator.cpp
	src/SphereGenerator.hpp
	src/SphereGenerator.cpp
	src/shader.cpp
	src/shader.hpp
	src/controls.cpp
	src/controls.hpp
	src/texture.cpp
	src/texture.hpp
	src/Mesh.hpp
	src/Mesh.cpp
	src/GLError.h
	src/GLError.cpp
	src/RenderState.hpp
	src/RenderState.cpp
	
    shaders/TerrainGenerator.vertexshader
    shaders/TerrainGenerator.geometryshader
    shaders/TerrainGenerator.fragmentshader
    shaders/TerrainGeneratorNormals.geometryshader
    shaders/TerrainGeneratorNormals.fragmentshader
    shaders/AtmosphericScattering.vertexshader
    shaders/AtmosphericScattering.fragmentshader
)
target_link_libraries(PlanetGenerator
	${ALL_LIBS}
	SOIL
)
set_target_properties(PlanetGenerator PROPERTIES COMPILE_DEFINITIONS "USE_ASSIMP")
# Xcode and Visual working directories
set_target_properties(PlanetGenerator PROPERTIES
	XCODE_ATTRIBUTE_CONFIGURATION_BUILD_DIR
	"${CMAKE_CURRENT_SOURCE_DIR}/src/")
set_target_properties(PlanetGenerator
	PROPERTIES
	XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS "/System/Library/Frameworks/OpenGL.framework/Versions/Current/")
create_target_launcher(PlanetGenerator WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/")

SOURCE_GROUP(src REGULAR_EXPRESSION ".*/src/.*" )
SOURCE_GROUP(shaders REGULAR_EXPRESSION ".*/.*shader$" )

if (NOT ${CMAKE_GENERATOR} MATCHES "Xcode" )

elseif (${CMAKE_GENERATOR} MATCHES "Xcode" )
endif (NOT ${CMAKE_GENERATOR} MATCHES "Xcode" )

