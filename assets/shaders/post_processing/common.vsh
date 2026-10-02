#version 330 core

const highp vec2 VERTICES[] = vec2[](
    vec2(0.0, 0.0),
    vec2(2.0, 0.0),
    vec2(0.0, 2.0));

out vec2 vTexCoord;

void main() {
    vTexCoord = VERTICES[gl_VertexID];
    gl_Position = vec4(VERTICES[gl_VertexID] * 2.0 - 1.0, 0.0, 1.0);
}
