#version 330 core

in vec3 positionInWorldSpace;
in vec3 normalInCameraSpace;
in vec3 lightDirectionInCameraSpace;
in float vertexSlope;

out vec4 color;

uniform vec3 lightDirectionInWorldSpace;
uniform vec3 lightColor;
uniform sampler2D heightSlopeBasedColorMap;

const float LightPower = 50000.0f;

float map(float value, float inMin, float inMax, float outMin, float outMax) {
    return outMin + (outMax - outMin) * (value - inMin) / (inMax - inMin);
}

vec3 calculateLight(vec3 lightDirection, float power, vec3 color, vec3 lightDirectionInCameraSpace) {
    float distance = length(lightDirection);
    power /= distance * distance;

    vec3 n = normalize(normalInCameraSpace);
    vec3 l = normalize(lightDirectionInCameraSpace);
    float ndotL = clamp(dot(n, l), 0, 1);

    return power * ndotL * color;
}

void main() {
    float heightCoordinate = clamp(map(length(positionInWorldSpace), 40, 80, 0, 1), 0, 1);
    float slopeCoordinate = clamp(map(log(vertexSlope + 1), 0, log(3.0), 0, 1), 0, 1);

    vec2 textureCoordinates = vec2(heightCoordinate, slopeCoordinate);
    vec4 MaterialDiffuseColor = texture(heightSlopeBasedColorMap, textureCoordinates);
    vec4 MaterialAmbientColor = vec4(0.0, 0.0, 0.0, 1.0) * MaterialDiffuseColor;

    vec3 diffuseLight = calculateLight(lightDirectionInWorldSpace, LightPower, lightColor, lightDirectionInCameraSpace);

    color = MaterialAmbientColor + MaterialDiffuseColor * vec4(diffuseLight, 1);
}
