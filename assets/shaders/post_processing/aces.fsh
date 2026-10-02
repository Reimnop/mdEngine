#version 330 core

out vec4 oFragColor;

in vec2 vTexCoord;

uniform float uExposure;
uniform sampler2D uTexture;

vec3 acesFilm(vec3 x) {
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

void main() {
    vec4 color = texture(uTexture, vTexCoord);
    color.rgb *= uExposure;
    color.rgb = acesFilm(color.rgb);
    oFragColor = color;
}
