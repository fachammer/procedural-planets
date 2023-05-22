#version 330 core

layout(location = 0) in vec3 vertexPositionInModelSpace;

out vec3 positionInWorldSpace;

uniform mat4 modelViewProjectionMatrix;
uniform mat4 modelMatrix;

void main() {
    positionInWorldSpace = (modelMatrix * vec4(vertexPositionInModelSpace, 1)).xyz;
    gl_Position = modelViewProjectionMatrix * vec4(vertexPositionInModelSpace, 1);
}
