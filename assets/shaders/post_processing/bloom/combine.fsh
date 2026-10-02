#version 330 core

out vec4 oFragColor;

in vec2 vTexCoord;

uniform sampler2D uSource;
uniform sampler2D uBloom;
uniform vec3 uTint;

void main() {
    vec3 source = texture(uSource, vTexCoord).rgb;
    vec3 bloom = texture(uBloom, vTexCoord).rgb;
    oFragColor = vec4(source + bloom * uTint, 1.0);
}
