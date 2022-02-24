#version 450

layout(location = 0) in vec2 position;
layout(location = 1) in vec3 color;

layout(push_constant) uniform Push {
    vec2 offset;
    vec3 color;
} push;

// Executed once for each vertex we provide
void main() {
    // Input: Vertex from Input Assembler stage
    // Output: Position (-1, -1) -> (1, 1). Assign to gl_Position.
    // gl_VertexIndex is the index of the current vertex.
    gl_Position = vec4(position + push.offset, 0.0, 1.0);
}
