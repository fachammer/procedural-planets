#version 330 core

layout(location = 0) in vec3 vertexPositionInModelSpace;

out vec3 positionInWorldSpace;
out vec3 normalInCameraSpace;
out vec3 lightDirectionInCameraSpace;
out float vertexSlope;

uniform mat4 modelViewProjectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;
uniform vec3 lightDirectionInWorldSpace;
uniform float maxNegativeHeight;
uniform float maxPositiveHeight;
uniform vec3 noiseOffset;

// psrdnoise (c) Stefan Gustavson and Ian McEwan,
// ver. 2021-12-02, published under the MIT license:
// https://github.com/stegu/psrdnoise/

vec4 permute(vec4 i) {
    vec4 im = mod(i, 289.0);
    return mod(((im * 34.0) + 10.0) * im, 289.0);
}

float psrdnoise(vec3 x, vec3 period, float alpha, out vec3 gradient) {
    const mat3 M = mat3(0.0, 1.0, 1.0, 1.0, 0.0, 1.0, 1.0, 1.0, 0.0);
    const mat3 Mi = mat3(-0.5, 0.5, 0.5, 0.5, -0.5, 0.5, 0.5, 0.5, -0.5);
    vec3 uvw = M * x;
    vec3 i0 = floor(uvw), f0 = fract(uvw);
    vec3 g_ = step(f0.xyx, f0.yzz), l_ = 1.0 - g_;
    vec3 g = vec3(l_.z, g_.xy), l = vec3(l_.xy, g_.z);
    vec3 o1 = min(g, l), o2 = max(g, l);
    vec3 i1 = i0 + o1, i2 = i0 + o2, i3 = i0 + vec3(1.0);
    vec3 v0 = Mi * i0, v1 = Mi * i1, v2 = Mi * i2, v3 = Mi * i3;
    vec3 x0 = x - v0, x1 = x - v1, x2 = x - v2, x3 = x - v3;
    if(any(greaterThan(period, vec3(0.0)))) {
        vec4 vx = vec4(v0.x, v1.x, v2.x, v3.x);
        vec4 vy = vec4(v0.y, v1.y, v2.y, v3.y);
        vec4 vz = vec4(v0.z, v1.z, v2.z, v3.z);
        if(period.x > 0.0)
            vx = mod(vx, period.x);
        if(period.y > 0.0)
            vy = mod(vy, period.y);
        if(period.z > 0.0)
            vz = mod(vz, period.z);
        i0 = floor(M * vec3(vx.x, vy.x, vz.x) + 0.5);
        i1 = floor(M * vec3(vx.y, vy.y, vz.y) + 0.5);
        i2 = floor(M * vec3(vx.z, vy.z, vz.z) + 0.5);
        i3 = floor(M * vec3(vx.w, vy.w, vz.w) + 0.5);
    }
    vec4 hash = permute(permute(permute(vec4(i0.z, i1.z, i2.z, i3.z)) + vec4(i0.y, i1.y, i2.y, i3.y)) + vec4(i0.x, i1.x, i2.x, i3.x));
    vec4 theta = hash * 3.883222077;
    vec4 sz = hash * -0.006920415 + 0.996539792;
    vec4 psi = hash * 0.108705628;
    vec4 Ct = cos(theta), St = sin(theta);
    vec4 sz_prime = sqrt(1.0 - sz * sz);
    vec4 gx, gy, gz;
    if(alpha != 0.0) {
        vec4 px = Ct * sz_prime, py = St * sz_prime, pz = sz;
        vec4 Sp = sin(psi), Cp = cos(psi), Ctp = St * Sp - Ct * Cp;
        vec4 qx = mix(Ctp * St, Sp, sz), qy = mix(-Ctp * Ct, Cp, sz);
        vec4 qz = -(py * Cp + px * Sp);
        vec4 Sa = vec4(sin(alpha)), Ca = vec4(cos(alpha));
        gx = Ca * px + Sa * qx;
        gy = Ca * py + Sa * qy;
        gz = Ca * pz + Sa * qz;
    } else {
        gx = Ct * sz_prime;
        gy = St * sz_prime;
        gz = sz;
    }
    vec3 g0 = vec3(gx.x, gy.x, gz.x), g1 = vec3(gx.y, gy.y, gz.y);
    vec3 g2 = vec3(gx.z, gy.z, gz.z), g3 = vec3(gx.w, gy.w, gz.w);
    vec4 w = 0.5 - vec4(dot(x0, x0), dot(x1, x1), dot(x2, x2), dot(x3, x3));
    w = max(w, 0.0);
    vec4 w2 = w * w, w3 = w2 * w;
    vec4 gdotx = vec4(dot(g0, x0), dot(g1, x1), dot(g2, x2), dot(g3, x3));
    float n = dot(w3, gdotx);
    vec4 dw = -6.0 * w2 * gdotx;
    vec3 dn0 = w3.x * g0 + dw.x * x0;
    vec3 dn1 = w3.y * g1 + dw.y * x1;
    vec3 dn2 = w3.z * g2 + dw.z * x2;
    vec3 dn3 = w3.w * g3 + dw.w * x3;
    gradient = 39.5 * (dn0 + dn1 + dn2 + dn3);
    return 39.5 * n;
}

float map(float value, float inMin, float inMax, float outMin, float outMax) {
    return outMin + (outMax - outMin) * (value - inMin) / (inMax - inMin);
}

float noise(vec3 position, out vec3 gradient) {
    return psrdnoise(position + noiseOffset, vec3(100), 0, gradient);
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
    vec3 normalInModelSpace;
    vec3 positionInModelSpace = displacedPosition(vertexPositionInModelSpace, -maxNegativeHeight, maxPositiveHeight, normalInModelSpace, vertexSlope);

    positionInWorldSpace = (modelMatrix * vec4(positionInModelSpace, 1)).xyz;
    lightDirectionInCameraSpace = (viewMatrix * vec4(-lightDirectionInWorldSpace, 0)).xyz;
    normalInCameraSpace = (viewMatrix * modelMatrix * vec4(normalInModelSpace, 0)).xyz;
    gl_Position = modelViewProjectionMatrix * vec4(positionInModelSpace, 1);
}
