#version 410 core

layout(vertices = 4) out;

void main() {
    // Pass through each control point
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

    // Only one invocation sets the tessellation levels for the entire patch.
    if (gl_InvocationID == 0) {
        // Set inner and outer tessellation levels.
        gl_TessLevelInner[0] = 4.0;
        gl_TessLevelInner[1] = 4.0;
        gl_TessLevelOuter[0] = 4.0;
        gl_TessLevelOuter[1] = 4.0;
        gl_TessLevelOuter[2] = 4.0;
        gl_TessLevelOuter[3] = 4.0;
    }
}
