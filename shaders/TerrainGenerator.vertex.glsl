#version 330 core

layout(location = 0) in vec3 vertexPosition_modelspace;

out vec3 positionInWorldSpace;
out vec3 normalInCameraSpace;
out vec3 lightDirectionInCameraSpace;
out float vertexSlope;

uniform mat4 modelViewProjectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;
uniform vec3 lightPositionInWorldSpace;
uniform float maxNegativeHeight;
uniform float maxPositiveHeight;
uniform vec3 noiseOffset;

//  Simplex 3D Noise
//  by Ian McEwan, Ashima Arts
vec4 permute(vec4 x) {
    return mod(((x * 34.0) + 1.0) * x, 289.0);
}
vec4 taylorInvSqrt(vec4 r) {
    return 1.79284291400159 - 0.85373472095314 * r;
}

float snoise(vec3 v, out vec3 gradient) {
    const vec2 C = vec2(1.0 / 6.0, 1.0 / 3.0);
    const vec4 D = vec4(0.0, 0.5, 1.0, 2.0);

    v += noiseOffset;

    // First corner
    vec3 i = floor(v + dot(v, C.yyy));
    vec3 x0 = v - i + dot(i, C.xxx);

    // Other corners
    vec3 g = step(x0.yzx, x0.xyz);
    vec3 l = 1.0 - g;
    vec3 i1 = min(g.xyz, l.zxy);
    vec3 i2 = max(g.xyz, l.zxy);

    //  x0 = x0 - 0. + 0.0 * C
    vec3 x1 = x0 - i1 + 1.0 * C.xxx;
    vec3 x2 = x0 - i2 + 2.0 * C.xxx;
    vec3 x3 = x0 - 1. + 3.0 * C.xxx;

    // Permutations
    i = mod(i, 289.0);
    vec4 p = permute(permute(permute(i.z + vec4(0.0, i1.z, i2.z, 1.0)) + i.y + vec4(0.0, i1.y, i2.y, 1.0)) + i.x + vec4(0.0, i1.x, i2.x, 1.0));

    // Gradients
    // ( N*N points uniformly over a square, mapped onto an octahedron.)
    float n_ = 1.0 / 7.0; // N=7
    vec3 ns = n_ * D.wyz - D.xzx;

    vec4 j = p - 49.0 * floor(p * ns.z * ns.z);  //  mod(p,N*N)

    vec4 x_ = floor(j * ns.z);
    vec4 y_ = floor(j - 7.0 * x_);    // mod(j,N)

    vec4 x = x_ * ns.x + ns.yyyy;
    vec4 y = y_ * ns.x + ns.yyyy;
    vec4 h = 1.0 - abs(x) - abs(y);

    vec4 b0 = vec4(x.xy, y.xy);
    vec4 b1 = vec4(x.zw, y.zw);

    vec4 s0 = floor(b0) * 2.0 + 1.0;
    vec4 s1 = floor(b1) * 2.0 + 1.0;
    vec4 sh = -step(h, vec4(0.0));

    vec4 a0 = b0.xzyw + s0.xzyw * sh.xxyy;
    vec4 a1 = b1.xzyw + s1.xzyw * sh.zzww;

    vec3 p0 = vec3(a0.xy, h.x);
    vec3 p1 = vec3(a0.zw, h.y);
    vec3 p2 = vec3(a1.xy, h.z);
    vec3 p3 = vec3(a1.zw, h.w);

    //Normalise gradients
    vec4 norm = taylorInvSqrt(vec4(dot(p0, p0), dot(p1, p1), dot(p2, p2), dot(p3, p3)));
    p0 *= norm.x;
    p1 *= norm.y;
    p2 *= norm.z;
    p3 *= norm.w;

    // Mix final noise value
    vec4 m = max(0.6 - vec4(dot(x0, x0), dot(x1, x1), dot(x2, x2), dot(x3, x3)), 0.0);
    vec4 m2 = m * m;
    vec4 m4 = m2 * m2;

    vec4 pdotx = vec4(dot(p0, x0), dot(p1, x1), dot(p2, x2), dot(p3, x3));

    vec4 temp = m2 * m * pdotx;
    gradient = -8.0 * (temp.x * x0 + temp.y * x1 + temp.z * x2 + temp.w * x3);
    gradient += m4.x * p0 + m4.y * p1 + m4.z * p2 + m4.w * p3;
    gradient *= 42.0;

    return 42.0 * dot(m4, pdotx);
}

float map(float value, float inMin, float inMax, float outMin, float outMax) {
    return outMin + (outMax - outMin) * (value - inMin) / (inMax - inMin);
}

float noise(vec3 position, out vec3 gradient) {
    return snoise(position, gradient);
}

float smax(float a, float b, float k, out float h) {
    float res = exp(k * a) + exp(k * b);
    float result = log(res) / k;
    h = clamp(0.5 + 0.5 * (a - b) / 5, 0.0, 1.0);
    return result;
}

float[] amplitudes = float[](1, 4, 4, 3, 1, 1, 0.5);
float[] frequencies = float[](4 / 1000.0, 8 / 1000.0, 16 / 1000.0, 32 / 1000.0, 64 / 1000.0, 128 / 1000.0, 256 / 1000.0);
float elevation(vec3 position, float minElevation, float maxElevation, out vec3 gradient) {

    float totalElevation = 0;
    gradient = vec3(0, 0, 0);
    float totalAmplitude = 0;
    for(int i = 0; i < amplitudes.length(); i++) {
        vec3 innerGradient;
        totalElevation += amplitudes[i] * noise(position * frequencies[i], innerGradient);
        gradient += amplitudes[i] * frequencies[i] * innerGradient;
        totalAmplitude += amplitudes[i];
    }

    gradient *= (maxElevation - minElevation) / (4 * totalAmplitude);
    float elevationValue = map(totalElevation, -totalAmplitude, totalAmplitude, minElevation, maxElevation);

    float threshold = 0;
    float interpolationFactor;
    elevationValue = smax(elevationValue, threshold, 3, interpolationFactor);
    gradient = mix(vec3(0, 0, 0), gradient, interpolationFactor);
    return elevationValue;
}

vec3 normal(vec3 position, vec3 gradient, float elevation, out float slope) {
    vec3 unitPosition = normalize(position);
    float radialGradientComponent = length(gradient);
    slope = radialGradientComponent;
    vec3 radialGradient = radialGradientComponent * unitPosition;
    vec3 tangentialGradient = gradient - radialGradient;
    vec3 unnormalizedNormal = position - elevation * tangentialGradient;
    return normalize(unnormalizedNormal);
}

vec3 displacedPosition(vec3 position, float minElevation, float maxElevation, out vec3 displacedNormal, out float slope) {
    vec3 gradient;
    float elevation = elevation(position, minElevation, maxElevation, gradient);
    vec3 newPosition = position * (1 + elevation / length(position));
    displacedNormal = normal(position, gradient, (maxElevation - minElevation) / 2, slope);
    return newPosition;
}

void main() {
    vec3 normal_modelspace;
    vec3 positionInModelSpace = displacedPosition(vertexPosition_modelspace, -maxNegativeHeight, maxPositiveHeight, normal_modelspace, vertexSlope);
    gl_Position = modelViewProjectionMatrix * vec4(positionInModelSpace, 1);

    positionInWorldSpace = (modelMatrix * vec4(positionInModelSpace, 1)).xyz;

    vec3 vertexPosition_cameraspace = (viewMatrix * modelMatrix * vec4(positionInModelSpace, 1)).xyz;
    vec3 eyeDirection_cameraspace = vec3(0, 0, 0) - vertexPosition_cameraspace;

    vec3 LightPosition_cameraspace = (viewMatrix * vec4(lightPositionInWorldSpace, 1)).xyz;
    lightDirectionInCameraSpace = LightPosition_cameraspace + eyeDirection_cameraspace;

    normalInCameraSpace = (viewMatrix * modelMatrix * vec4(normal_modelspace, 0)).xyz;
}
