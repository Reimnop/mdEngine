#version 330 core

const float EPSILON = 0.0001;

out vec4 oFragColor;

in vec2 vTexCoord;

uniform sampler2D uTexture;
uniform float uThreshold;
uniform float uKnee;

// soft knee thresholding function
vec3 applyThreshold(vec3 color, float threshold, float knee) {
    float brightness = max(max(color.r, color.g), color.b);
    float softness = clamp(brightness - threshold + knee, 0.0, 2.0 * knee);
    softness = softness * softness / (4.0 * knee + EPSILON);
    float multiplier = max(brightness - threshold, softness) / max(brightness, EPSILON);
    return color * multiplier;
}

void main() {
    vec3 color = texture(uTexture, vTexCoord).rgb;
    oFragColor = vec4(applyThreshold(color, uThreshold, uKnee), 1.0);
}
