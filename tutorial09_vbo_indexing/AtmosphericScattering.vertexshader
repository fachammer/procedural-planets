//
//  TerrainGenerator.vertexshader
//  Tutorials
//
//  Created by Fabian Achammer on 10.12.15.
//
//

#version 330 core

layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in vec3 vertexNormal_modelspace;

out vec3 position_worldspace;

uniform mat4 MVP;
uniform mat4 M;

void main() {
    gl_Position =  MVP * vec4(vertexPosition_modelspace, 1);
    position_worldspace = (M * vec4(vertexPosition_modelspace, 1)).xyz;
}
