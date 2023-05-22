#version 330 core

in vec3 positionInWorldSpace;
in vec3 normalInCameraSpace;
in vec3 lightDirectionInCameraSpace;
in float vertexSlope;

out vec4 color;

uniform vec3 lightColor;
uniform sampler2D heightSlopeBasedColorMap;
uniform float lightPower;
uniform float baseRadius;
uniform float maxNegativeHeight;
uniform float maxPositiveHeight;

float map(float value, float inMin, float inMax, float outMin, float outMax) {
    return outMin + (outMax - outMin) * (value - inMin) / (inMax - inMin);
}

vec3 calculateLight(float power, vec3 color, vec3 lightDirectionInCameraSpace) {
    vec3 n = normalize(normalInCameraSpace);
    vec3 l = normalize(lightDirectionInCameraSpace);
    float ndotL = clamp(dot(n, l), 0, 1);

    return power * ndotL * color;
}

void main() {
    float heightCoordinate = clamp(map(length(positionInWorldSpace), baseRadius, baseRadius + maxPositiveHeight - 10, 0, 1), 0, 1);
    float slopeCoordinate = clamp(map(log(vertexSlope + 1), 0, log(3.0), 0, 1), 0, 1);

    vec2 textureCoordinates = vec2(heightCoordinate, slopeCoordinate);
    vec4 materialDiffuseColor = texture(heightSlopeBasedColorMap, textureCoordinates);
    vec4 materialAmbientColor = 0.03 * materialDiffuseColor;

    vec3 diffuseLight = calculateLight(lightPower, lightColor, lightDirectionInCameraSpace);

    color = materialAmbientColor + materialDiffuseColor * vec4(diffuseLight, 1);
}
