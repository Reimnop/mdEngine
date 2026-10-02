#version 330 core

out vec4 oFragColor;

in vec2 vTexCoord;

uniform sampler2D uTexture;

vec3 sample13Tap(sampler2D mip, vec2 uv, vec2 radius) {
    // A B C
    //  J K
    // D E F
    //  L M
    // G H I
    vec3 a = texture(mip, uv + vec2(-2.0, -2.0) * radius).rgb;
    vec3 b = texture(mip, uv + vec2( 0.0, -2.0) * radius).rgb;
    vec3 c = texture(mip, uv + vec2( 2.0, -2.0) * radius).rgb;
    vec3 d = texture(mip, uv + vec2(-2.0,  0.0) * radius).rgb;
    vec3 e = texture(mip, uv + vec2( 0.0,  0.0) * radius).rgb;
    vec3 f = texture(mip, uv + vec2( 2.0,  0.0) * radius).rgb;
    vec3 g = texture(mip, uv + vec2(-2.0,  2.0) * radius).rgb;
    vec3 h = texture(mip, uv + vec2( 0.0,  2.0) * radius).rgb;
    vec3 i = texture(mip, uv + vec2( 2.0,  2.0) * radius).rgb;
    vec3 j = texture(mip, uv + vec2(-1.0, -1.0) * radius).rgb;
    vec3 k = texture(mip, uv + vec2( 1.0, -1.0) * radius).rgb;
    vec3 l = texture(mip, uv + vec2(-1.0,  1.0) * radius).rgb;
    vec3 m = texture(mip, uv + vec2( 1.0,  1.0) * radius).rgb;

    return e * 0.125 +
        (a + c + g + i) * 0.03125 +
        (b + d + f + h) * 0.0625 +
        (j + k + l + m) * 0.125;
}

void main() {
    vec2 pxSize = 1.0 / vec2(textureSize(uTexture, 0));
    oFragColor = vec4(sample13Tap(uTexture, vTexCoord, pxSize), 1.0);
}
