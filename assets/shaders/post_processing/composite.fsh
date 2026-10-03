#version 330 core

out vec4 oFragColor;

in vec2 vTexCoord;

uniform sampler2D uTexture;

float toLinear(float value) {
    return value <= 0.04045
        ? value / 12.92
        : pow((value + 0.055) / 1.055, 2.4);
}

float toGamma(float value) {
    return value <= 0.0031308
        ? value * 12.92
        : 1.055 * pow(value, 1.0 / 2.4) - 0.055;
}

vec3 toLinear(vec3 color) {
    return vec3(toLinear(color.r), toLinear(color.g), toLinear(color.b));
}

vec3 toGamma(vec3 color) {
    return vec3(toGamma(color.r), toGamma(color.g), toGamma(color.b));
}

void main() {
    vec4 color = texture(uTexture, vTexCoord);
    color.rgb = toGamma(color.rgb);
    oFragColor = color;
}
