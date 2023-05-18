//
//  TerrainGenerator.fragmentshader
//  Tutorials
//
//  Created by Fabian Achammer on 10.12.15.
//
//

#version 330 core

// Interpolated values from the vertex shaders
in vec2 UV;
in vec3 Position_worldspace;
in vec3 Normal_cameraspace;
in vec3 EyeDirection_cameraspace;
in vec3 LightDirection_cameraspace;
in vec3 Vertex_color;
in float Vertex_slope;

// Ouput data
out vec4 color;

uniform mat4 MV;
uniform mat4 V;
uniform vec3 LightPosition_worldspace;
uniform sampler2D heightSlopeBasedColorMap;
uniform float maxNegativeHeight;
uniform float maxPositiveHeight;
uniform float baseRadius;
uniform vec3 cameraPosition;

const vec3 lightColor = vec3(1, 1, 1);

const float LightPower = 50000.0f;

float map(float value, float inMin, float inMax, float outMin, float outMax) {
    return outMin + (outMax - outMin) * (value - inMin) / (inMax - inMin);
}

vec3 calcLight(vec3 position, float power, vec3 color, vec3 lightDirection_cameraSpace){
	vec3 lightDirection = Position_worldspace - position;
    float distance = length(lightDirection);
    power /= distance * distance;
    
    vec3 n = normalize( Normal_cameraspace );
    vec3 l = normalize( lightDirection_cameraSpace );
    float ndotL = clamp( dot( n,l ), 0,1 );

    return power * ndotL * color;
}

void main() {
    // Material properties
    vec2 heightSlopeBasedColorMapSize = textureSize(heightSlopeBasedColorMap, 0);
    float heightCoordinate = clamp(map(length(Position_worldspace),
                                       40,
                                       80, 0, 1), 0, 1);
    
    float slopeCoordinate = clamp(map(log(Vertex_slope + 1), 0, log(3.0), 0, 1), 0, 1);
    vec2 textureCoordinates = vec2(heightCoordinate, slopeCoordinate);
    vec4 MaterialDiffuseColor = texture(heightSlopeBasedColorMap, textureCoordinates);
    vec4 MaterialAmbientColor = vec4(0.0, 0.0, 0.0, 1.0) * MaterialDiffuseColor;

    vec3 diffuseLight = calcLight(LightPosition_worldspace, LightPower, lightColor, LightDirection_cameraspace);

    color = MaterialAmbientColor + MaterialDiffuseColor * vec4(diffuseLight, 1);
}
