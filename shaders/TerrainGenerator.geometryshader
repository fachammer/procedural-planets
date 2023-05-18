//
//  TerrainGenerator.c
//  Tutorials
//
//  Created by Fabian Achammer on 10.12.15.
//
//

#version 330 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in vec3[] position_worldspace;
in vec3[] normal_cameraspace;
in vec3[] lightDirection_cameraspace;
in vec3[] position_modelspace;
in float[] vertex_slope;

out vec3 Position_worldspace;
out vec3 Normal_cameraspace;
out vec3 LightDirection_cameraspace;
out float Vertex_slope;

uniform mat4 MVP;
uniform mat4 M;
uniform mat4 V;
uniform float atmosphereRadius;

void main() {
    for(int i = 0; i < 3; i++) {
        Position_worldspace = position_worldspace[i];
        Normal_cameraspace = normal_cameraspace[i];
        LightDirection_cameraspace = lightDirection_cameraspace[i];
        Vertex_slope = vertex_slope[i];
        gl_Position = MVP * vec4(position_modelspace[i], 1);
        EmitVertex();
    }
    
    EndPrimitive();
}
