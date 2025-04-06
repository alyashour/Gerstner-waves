#version 410 core

layout(vertices = 4) out;

// Input from vertex shader
in vec2 uv_vs[];

// Output to tess eval shader
out vec2 uv_tcs[];

uniform float outerTess;
uniform float innerTess;

void main() {
    // Pass through the control point
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    uv_tcs[gl_InvocationID] = uv_vs[gl_InvocationID];

    // Only one invocation sets the tessellation levels
    if (gl_InvocationID == 0) {
        gl_TessLevelInner[0] = innerTess;
        gl_TessLevelInner[1] = innerTess;
        gl_TessLevelOuter[0] = outerTess;
        gl_TessLevelOuter[1] = outerTess;
        gl_TessLevelOuter[2] = outerTess;
        gl_TessLevelOuter[3] = outerTess;
    }
}
