#version 450

layout(location = 0) in vec3 fragColor;

// Declare output variable
layout(location = 0) out vec4 outColor;


layout(push_constant) uniform Push {
    mat4 transform; // projection * view * model
    mat4 normalMatrix; // normal to world
} push;

void main() {
    // RGBA
    outColor = vec4(fragColor, 1.0);
}
