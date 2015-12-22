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
	external/AntTweakBar-1.16/include/
	external/glfw-3.0.3/include/GLFW/
	external/glm-0.9.4.0/
	external/glew-1.9.0/include/
	external/assimp-3.0.1270/include/
	external/Soil/src/
	#external/bullet-2.81-rev2613/src/
	.
)

set(ALL_LIBS
	${OPENGL_LIBRARY}
	GLFW_303
	GLEW_190
)

add_definitions(
	-DTW_STATIC
	-DTW_NO_LIB_PRAGMA
	-DTW_NO_DIRECT3D
	-DGLEW_STATIC
	-D_CRT_SECURE_NO_WARNINGS
)

# Tutorial 9 - AssImp model loading
add_executable(tutorial09_AssImp
	tutorial09_vbo_indexing/tutorial09_AssImp.cpp
	tutorial09_vbo_indexing/SphereGenerator.hpp
	tutorial09_vbo_indexing/SphereGenerator.cpp
	common/shader.cpp
	common/shader.hpp
	common/controls.cpp
	common/controls.hpp
	common/texture.cpp
	common/texture.hpp
	common/objloader.cpp
	common/objloader.hpp
	common/vboindexer.cpp
	common/vboindexer.hpp
	common/Mesh.hpp
	common/Mesh.cpp
	common/GLError.h
	common/GLError.cpp
        common/tangentspace.cpp
        common/tangentspace.hpp
	common/RenderState.hpp
	common/RenderState.cpp
	
    tutorial09_vbo_indexing/TerrainGenerator.vertexshader
    tutorial09_vbo_indexing/TerrainGenerator.geometryshader
    tutorial09_vbo_indexing/TerrainGenerator.fragmentshader
    tutorial09_vbo_indexing/TerrainGeneratorNormals.geometryshader
    tutorial09_vbo_indexing/TerrainGeneratorNormals.fragmentshader
)
target_link_libraries(tutorial09_AssImp
	${ALL_LIBS}
	assimp
	ANTTWEAKBAR_116_OGLCORE_GLFW
	SOIL
)
set_target_properties(tutorial09_AssImp PROPERTIES COMPILE_DEFINITIONS "USE_ASSIMP")
# Xcode and Visual working directories
set_target_properties(tutorial09_AssImp PROPERTIES
	XCODE_ATTRIBUTE_CONFIGURATION_BUILD_DIR
	"${CMAKE_CURRENT_SOURCE_DIR}/tutorial09_vbo_indexing/")
set_target_properties(tutorial09_AssImp
	PROPERTIES
	XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS "/System/Library/Frameworks/OpenGL.framework/Versions/Current/")
create_target_launcher(tutorial09_AssImp WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/tutorial09_vbo_indexing/")

SOURCE_GROUP(common REGULAR_EXPRESSION ".*/common/.*" )
SOURCE_GROUP(shaders REGULAR_EXPRESSION ".*/.*shader$" )

if (NOT ${CMAKE_GENERATOR} MATCHES "Xcode" )

add_custom_command(
   TARGET SpongeBob POST_BUILD
   COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/tutorial09_vbo_indexing${CMAKE_EXECUTABLE_SUFFIX}" "${CMAKE_CURRENT_SOURCE_DIR}/tutorial09_vbo_indexing/"
)
add_custom_command(
   TARGET tutorial09_AssImp POST_BUILD
   COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/tutorial09_AssImp${CMAKE_EXECUTABLE_SUFFIX}" "${CMAKE_CURRENT_SOURCE_DIR}/tutorial09_vbo_indexing/"
)
add_custom_command(
   TARGET tutorial09_several_objects POST_BUILD
   COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/tutorial09_several_objects${CMAKE_EXECUTABLE_SUFFIX}" "${CMAKE_CURRENT_SOURCE_DIR}/tutorial09_vbo_indexing/"
)

elseif (${CMAKE_GENERATOR} MATCHES "Xcode" )
endif (NOT ${CMAKE_GENERATOR} MATCHES "Xcode" )

