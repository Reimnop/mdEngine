#version 330 core

out vec4 oFragColor;

in vec2 vTexCoord;

uniform sampler2D uTexture;
uniform float uSampleScale;

vec3 sampleTent(sampler2D mip, vec2 uv, vec2 radius) {
    // A B C
    // D E F
    // G H I
    vec3 a = texture(mip, uv + vec2(-1.0, -1.0) * radius).rgb;
    vec3 b = texture(mip, uv + vec2( 0.0, -1.0) * radius).rgb;
    vec3 c = texture(mip, uv + vec2( 1.0, -1.0) * radius).rgb;
    vec3 d = texture(mip, uv + vec2(-1.0,  0.0) * radius).rgb;
    vec3 e = texture(mip, uv + vec2( 0.0,  0.0) * radius).rgb;
    vec3 f = texture(mip, uv + vec2( 1.0,  0.0) * radius).rgb;
    vec3 g = texture(mip, uv + vec2(-1.0,  1.0) * radius).rgb;
    vec3 h = texture(mip, uv + vec2( 0.0,  1.0) * radius).rgb;
    vec3 i = texture(mip, uv + vec2( 1.0,  1.0) * radius).rgb;

    return (a + c + g + i) * 0.0625 +
        (b + d + f + h) * 0.125 +
        e * 0.25;
}

void main() {
    vec2 pxSize = 1.0 / vec2(textureSize(uTexture, 0));

    // output alpha is 1.0, accumulation into the target mip is done with additive blending
    oFragColor = vec4(sampleTent(uTexture, vTexCoord, pxSize * uSampleScale), 1.0);
}
