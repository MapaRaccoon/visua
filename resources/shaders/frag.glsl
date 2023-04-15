#version 330 core

in vec2 vPos;

out vec3 color;

vec3 hsv2rgb(vec3 c);

void main() {
    float hue = length(vPos);
    color = hsv2rgb(vec3(hue, 1.0f, 1.0f));
}

// All components are in the range [0…1], including hue.
// TODO: study this more
vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}
