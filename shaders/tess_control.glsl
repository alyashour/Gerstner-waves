#version 410 core

layout(vertices = 4) out;

uniform float outerTess;
uniform float innerTess;

void main() {
    // Pass through each control point
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

    // Only one invocation sets the tessellation levels for the entire patch.
    if (gl_InvocationID == 0) {
        // Set inner and outer tessellation levels.
        gl_TessLevelInner[0] = outerTess;
        gl_TessLevelInner[1] = outerTess;
        gl_TessLevelOuter[0] = outerTess;
        gl_TessLevelOuter[1] = outerTess;
        gl_TessLevelOuter[2] = innerTess;
        gl_TessLevelOuter[3] = innerTess;
    }
}
