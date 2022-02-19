#version 450

vec2 position[3] = vec2[] (
    vec2(0.0, -0.5),
    vec2(0.5, 0.5),
    vec2(-0.5, 0.5)
);

// Executed once for each vertex we have
void main() {
    // Input: Vertex from Input Assembler stage
    // Output: Position (-1, -1) -> (1, 1). Assign to gl_Position.
    // gl_VertexIndex is the index of the current vertex.
    gl_Position = vec4(position[gl_VertexIndex], 0.0, 1.0);
}
