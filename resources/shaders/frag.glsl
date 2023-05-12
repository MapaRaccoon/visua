#version 330 core

#define PI 3.1415926536

struct Parameters
{
    float wiggleOffset;
    float wiggleAmplitude;
    float wigglesPerRevolution;
    float wigglePhase;
    float normExponent;
};

in vec2 vPos;
out vec3 color;

uniform sampler1D tex;
uniform Parameters params;

vec3 hsv2rgb(vec3 c);
float pNorm(vec2 vec, float p);

// float wiggleOffset = 4.0;
// float wiggleScale = 0.2;
// float wigglesPerRevolution = 16;
// float wigglePhase = 0;
// float normExponent = 2.0/3;

void main() {
    float mag = pNorm(vPos, params.normExponent);
    float angle = atan(vPos.y, vPos.x);
    float hue = texture(
        tex, 
        mag / (
            params.wiggleOffset + params.wiggleAmplitude * sin(
                (angle + params.wigglePhase) * params.wigglesPerRevolution
            )
        )
    ).r;
    color = hsv2rgb(vec3(hue, 1, hue));
}

// All components are in the range [0…1], including hue.
// TODO: study this more
vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

float pNorm(vec2 vec, float p)
{
    return pow(pow(abs(vec.x), p) + pow(abs(vPos.y), p), 1/p);
}
