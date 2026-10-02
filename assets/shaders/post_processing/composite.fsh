#version 330 core

out vec4 oFragColor;

in vec2 vTexCoord;

uniform sampler2D uTexture;

void main() {
    vec4 color = texture(uTexture, vTexCoord);
    color.rgb = pow(color.rgb, vec3(1.0 / 2.2)); // linear -> gamma
    oFragColor = color;
}
