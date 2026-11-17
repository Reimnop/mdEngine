#version 330 core

layout(location = 0) out vec4 oFragColor;

in vec3 vNormal;
in vec2 vTexCoord;
in vec3 vFragPos;
in vec3 vVertexColor;
in vec3 vGouroudColor;

uniform mat4 uView;
uniform vec3 uColor;
uniform int uShadingMode; // 0 = normal, 1 = flat
uniform float uShininess;
uniform float uSpecularStrength;
uniform vec3 uLightDir;
uniform vec3 uLightColor;
uniform float uAmbient;
uniform bool uUseTexture;

uniform sampler2D uTexture;

void main() {
    vec4 textureColor = uUseTexture ? texture(uTexture, vTexCoord) : vec4(1.0);

    if (uShadingMode != 1){
        oFragColor = vec4(uColor * vVertexColor, 1.0) * textureColor;
        return;
    }

    vec3 light = length(uLightDir) > 0.0 ? normalize(uLightDir) : vec3(0.0, 1.0, 0.0);
    vec3 n = normalize(vNormal);
    vec3 diffuse = max(dot(n, light), 0.0) * uLightColor;
    vec3 viewDir = normalize(-vFragPos);
    vec3 reflectDir = reflect(transpose(inverse(mat3(uView))) * -light, transpose(inverse(mat3(uView))) * n);
    vec3 specular = pow(max(dot(viewDir, reflectDir), 0.0), uShininess) * uLightColor;
    oFragColor = vec4((diffuse + specular * uSpecularStrength + vec3(uAmbient)) * uColor, 1.0) * vec4(vVertexColor, 1.0) * textureColor;
}
