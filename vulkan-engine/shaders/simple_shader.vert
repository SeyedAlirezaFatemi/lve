#version 450

layout(location = 0) in vec2 position;
layout(location = 1) in vec3 color;

layout(location = 0) out vec3 fragColor;

// Executed once for each vertex we provide
void main() {
    // Input: Vertex from Input Assembler stage
    // Output: Position (-1, -1) -> (1, 1). Assign to gl_Position.
    // gl_VertexIndex is the index of the current vertex.
    gl_Position = vec4(position, 0.0, 1.0);
    fragColor = color;
}
