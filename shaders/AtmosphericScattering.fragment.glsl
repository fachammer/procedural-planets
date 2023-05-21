#version 330 core

in vec3 positionInWorldSpace;

// Ouput data
out vec4 color;

uniform vec3 cameraPositionInWorldSpace;
uniform vec3 lightPositionInWorldSpace;
uniform float atmosphereRadius;
uniform float baseRadius;

// adapted code from: https://www.shadertoy.com/view/lslXDr
// Written by GLtracy

// math const
const float PI = 3.14159265359;
const float MAX = 10000.0;

// scatter const
const float K_R = 0.166;
const float K_M = 0.0025;
float E = 14.3; 						// light intensity
const vec3 C_R = vec3(0.3, 0.7, 1.0); 	// 1 / wavelength ^ 4
const float G_M = -0.85;					// Mie g

float R = 84.0;
float R_INNER = 50.0;
float SCALE_H = 4.0 / (R - R_INNER);
float SCALE_L = 1.0 / (R - R_INNER);

const int NUM_OUT_SCATTER = 3;
const float FNUM_OUT_SCATTER = 3.0;

const int NUM_IN_SCATTER = 3;
const float FNUM_IN_SCATTER = 3.0;

// ray intersects sphere
// e = -b +/- sqrt( b^2 - c )
vec2 ray_vs_sphere(vec3 p, vec3 dir, float r) {
    float b = dot(p, dir);
    float c = dot(p, p) - r * r;

    float d = b * b - c;
    if(d < 0.0) {
        return vec2(MAX, -MAX);
    }
    d = sqrt(d);

    return vec2(-b - d, -b + d);
}

// Mie
// g : ( -0.75, -0.999 )
//      3 * ( 1 - g^2 )               1 + c^2
// F = ----------------- * -------------------------------
//      2 * ( 2 + g^2 )     ( 1 + g^2 - 2 * g * c )^(3/2)
float phase_mie(float g, float c, float cc) {
    float gg = g * g;

    float a = (1.0 - gg) * (1.0 + cc);

    float b = 1.0 + gg - 2.0 * g * c;
    b *= sqrt(b);
    b *= 2.0 + gg;

    return 1.5 * a / b;
}

// Reyleigh
// g : 0
// F = 3/4 * ( 1 + c^2 )
float phase_reyleigh(float cc) {
    return 0.75 * (1.0 + cc);
}

float density(vec3 p) {
    return exp(-(length(p) - R_INNER) * SCALE_H);
}

float optic(vec3 p, vec3 q) {
    vec3 step = (q - p) / FNUM_OUT_SCATTER;
    vec3 v = p + step * 0.5;

    float sum = 0.0;
    for(int i = 0; i < NUM_OUT_SCATTER; i++) {
        sum += density(v);
        v += step;
    }
    sum *= length(step) * SCALE_L;

    return sum;
}

vec3 in_scatter(vec3 o, vec3 dir, vec2 e, vec3 l) {
    float len = (e.y - e.x) / FNUM_IN_SCATTER;
    vec3 step = dir * len;
    vec3 p = o + dir * e.x;
    vec3 v = p + dir * (len * 0.5);

    vec3 sum = vec3(0.0);
    for(int i = 0; i < NUM_IN_SCATTER; i++) {
        vec2 f = ray_vs_sphere(v, l, R);
        vec3 u = v + l * f.y;

        float n = (optic(p, v) + optic(v, u)) * (PI * 4.0);

        sum += density(v) * exp(-n * (K_R * C_R + K_M));

        v += step;
    }
    sum *= len * SCALE_L;

    float c = dot(dir, -l);
    float cc = c * c;

    return sum * (K_R * C_R * phase_reyleigh(cc) + K_M * phase_mie(G_M, c, cc)) * E;
}

void main() {
    float lightPower = 50000;
    R = atmosphereRadius;
    R_INNER = baseRadius;
    SCALE_H = 4.0 / (R - R_INNER);
    SCALE_L = 1.0 / (R - R_INNER);
    vec3 lightDirection = lightPositionInWorldSpace - positionInWorldSpace;
    E = lightPower / length(lightDirection);

    vec3 eye = cameraPositionInWorldSpace;
    vec3 dir = normalize(positionInWorldSpace - cameraPositionInWorldSpace);

    vec2 e = ray_vs_sphere(eye, dir, atmosphereRadius);
    vec2 f = ray_vs_sphere(eye, dir, baseRadius);
    e.y = min(e.y, f.x);

    vec3 I = in_scatter(eye, dir, e, normalize(lightDirection));

    I = 1.0 - exp(-0.2 * I);

    color = vec4(I, 1.0);
}
