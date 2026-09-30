#version 330 core

layout(location = 0) out vec4 oFragColor;

in vec3 vNormal;
in vec2 vTexCoord;
in vec3 vGouroudColor;

uniform int uShadingMode; // 0: flat, 1: gouraud, 2: phong
uniform vec3 uColor;
uniform vec3 uLightDir;
uniform vec3 uLightColor;
uniform float uAmbient;

void main() {
    if (uShadingMode == 0) {
        oFragColor = vec4(uColor, 1.0);
        return;
    }

    if (uShadingMode == 1) {
        oFragColor = vec4(uColor * vGouroudColor, 1.0);
        return;
    }

    if (uShadingMode == 2) {
        vec3 light = length(uLightDir) > 0.0 ? normalize(uLightDir) : vec3(0.0, 1.0, 0.0);
        vec3 n = normalize(vNormal);
        float diffuse = max(dot(n, light), 0.0) + 0.1 * max(dot(n, -light), 0.0);
        oFragColor = vec4(uColor * (uAmbient + (1.0 - uAmbient) * diffuse * uLightColor), 1.0);
        return;
    }
}
