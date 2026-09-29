#version 330 core

layout(location = 0) out vec4 oFragColor;

in vec3 vNormal;
in vec2 vTexCoord;

void main() {
    oFragColor = vec4(vNormal, 1.0);
}