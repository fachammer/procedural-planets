if(MSVC AND NOT "${MSVC_VERSION}" LESS 1400)
	add_definitions( "/MP" )
endif()


add_definitions(
	-DTW_STATIC
	-DTW_NO_LIB_PRAGMA
	-DTW_NO_DIRECT3D
	-DGLEW_STATIC
	-D_CRT_SECURE_NO_WARNINGS
)

### GLFW ###

add_subdirectory (glfw-3.3.8)

include_directories(
	glfw-3.3.8/include/GLFW/
	glew-1.9.0/include/
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
set(OPENGL_LIBRARY
	${OPENGL_LIBRARY}
	-lGL -lGLU -lXrandr -lXext -lX11 -lrt
	${CMAKE_DL_LIBS}
	${GLFW_LIBRARIES}
)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
set(OPENGL_LIBRARY
	${OPENGL_LIBRARY}
	${CMAKE_DL_LIBS}
	${GLFW_LIBRARIES}
)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

### GLEW ###

set(GLEW_SOURCE
	glew-1.9.0/src/glew.c
)

set(GLEW_HEADERS
)


add_library( GLEW_190 STATIC
	${GLEW_SOURCE}
	${GLEW_INCLUDE}
)

target_link_libraries(GLEW_190
	${OPENGL_LIBRARY}
	${EXTRA_LIBS}
)

### SOIL ###
set(SOIL_SOURCE
	Soil/src/image_DXT.c
	Soil/src/image_helper.c
	Soil/src/SOIL.c
	Soil/src/stb_image_aug.c
)

set(SOIL_HEADERS
	Soil/src/image_DXT.h
	Soil/src/image_helper.h
	Soil/src/SOIL.h
	Soil/src/stb_image_aug.h
	Soil/src/stbi_DDS_aug.h
	Soil/src/stbi_DDS_aug_c.h
	)
	
include_directories(
	${CMAKE_SOURCE_DIR}/external/Soil/src/
)

add_library( SOIL STATIC
	${SOIL_SOURCE}
	${SOIL_HEADERS}
)
