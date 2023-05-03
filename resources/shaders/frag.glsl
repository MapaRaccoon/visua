#version 330 core

#define PI 3.1415926536

in vec2 vPos;
out vec3 color;

uniform sampler1D tex;

vec3 hsv2rgb(vec3 c);

void main() {
    float mag = length(vPos);
    //float hue = mag;
    //float sat = (sin(mag * 2 * PI * 5 ) + 1)/4;
    //float sat = 1;
    //color = hsv2rgb(vec3(hue, sat, 1.0f));
    float val = texture(tex, mag/4).r;
    color = hsv2rgb(vec3(val, 1, 1));
}

// All components are in the range [0…1], including hue.
// TODO: study this more
vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}
