#version 410 core
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

void main() {
    // Simply pass through each vertex of the triangle.
    for (int i = 0; i < 3; i++) {
        gl_Position = gl_in[i].gl_Position;
        EmitVertex();
    }
    EndPrimitive();
}
