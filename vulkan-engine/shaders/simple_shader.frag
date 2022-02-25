#version 450

// Declare output variable
layout (location = 0) out vec4 outColor;

layout(push_constant) uniform Push {
    mat2 transform;
    vec2 offset;
    vec3 color;
} push;

void main() {
    // RGBA
    outColor = vec4(push.color, 1.0);
}
