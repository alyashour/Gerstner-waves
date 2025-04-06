#version 410 core

layout(location = 0) in vec3 position;

uniform mat4 MVP;
uniform float texScale;
uniform vec2 texOffset;
uniform float time;

// Outputs to tess control shader
out vec2 uv_vs;

void main() {
    // Pass the vertex position as a vec4 to the next stage
    gl_Position = vec4(position, 1.0);
    
    // Calculate and pass UV coordinates
    uv_vs = (position.xz + texOffset + (time * 0.001))/texScale;
}
