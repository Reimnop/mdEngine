#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec3 aColor;

out vec3 vNormal;
out vec2 vTexCoord;
out vec3 vFragPos;
out vec3 vVertexColor;

uniform mat4 uMvp;
uniform mat4 uModel;
uniform mat4 uView;
uniform vec3 uColor;
uniform float uShininess;
uniform float uSpecularStrength;
uniform vec3 uLightDir;
uniform vec3 uLightColor;
uniform float uAmbient;

void main() {
    vNormal = transpose(inverse(mat3(uModel))) * aNormal;
    vTexCoord = aTexCoord;
    vFragPos = (uView * uModel * vec4(aPos, 1.0)).xyz;
    vVertexColor = aColor;
    gl_Position = uMvp * vec4(aPos, 1.0);
}
