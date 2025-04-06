#version 410 core

layout(location = 0) in vec3 position;

uniform mat4 MVP;

void main() {
    // Pass the vertex position as a vec4 to the next stage.
    gl_Position = vec4(position, 1.0);
}
