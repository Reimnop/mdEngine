#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

out vec3 vNormal;
out vec2 vTexCoord;
out vec3 vGouroudColor;

uniform mat4 uMvp;
uniform mat4 uModel;
uniform int uShadingMode; // 0: flat, 1: gouraud, 2: phong
uniform vec3 uColor;
uniform vec3 uLightDir;
uniform vec3 uLightColor;
uniform float uAmbient;

void main() {
    vNormal = transpose(inverse(mat3(uModel))) * aNormal;
    vTexCoord = aTexCoord;
    gl_Position = uMvp * vec4(aPos, 1.0);

    if (uShadingMode == 1) {
        vec3 light = length(uLightDir) > 0.0 ? normalize(uLightDir) : vec3(0.0, 1.0, 0.0);
        vec3 n = normalize(vNormal);
        float diffuse = max(dot(n, light), 0.0) + 0.3 * max(dot(n, -light), 0.0);
        vGouroudColor = uColor * (uAmbient + (1.0 - uAmbient) * diffuse * uLightColor);
    }
}
