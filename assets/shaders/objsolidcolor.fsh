#version 330 core

layout(location = 0) out vec4 oFragColor;

uniform vec3 uColor;

void main() {
    oFragColor = vec4(uColor, 1.0);
}
