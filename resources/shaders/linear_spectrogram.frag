#version 330 core

#define PI 3.1415926536

in vec2 vPos;
out vec3 color;

uniform sampler1D tex;

vec3 hsv2rgb(vec3 c);
vec3 fire(float val);

void main() {


    // get distance from bottom  of screen, between 0 to 1
    float pos = (vPos.y + 1) / 2;

    // tile
    pos = fract(pos * 4);

    // mirror
    pos = abs(0.5 - pos);

    // scale differently depending on frequencies
    if (pos < 0.1) {
        pos = (200 * 10 * pos) / 44100.0f;
    } else {
        pos = (pos - 0.1) / 0.9;
        pos = (200 + (10000 - 200) * pos) / 44100.0f;
    }

    float amp = texture(tex, pos).r;
    //if (amp < 0.5) amp = 0;
    amp = pow(amp, 2);
    color = fire(amp);
}

// All components are in the range [0…1], including hue.
// TODO: study this more
vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

// converts float in [0, 1] to fire color spectrum
vec3 fire(float val)
{
    return hsv2rgb(vec3((1 - val) * (1.0f/6.0f), 1, val));
}
