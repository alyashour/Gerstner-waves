#version 410 core

layout(quads, equal_spacing, cw) in;

// Input from tess control shader
in vec2 uv_tcs[];

// Output to geometry shader
out vec2 uv_tes;

uniform mat4 MVP;
uniform sampler2D distext;

void main() {
    // Interpolate positions
    vec4 p0 = gl_in[0].gl_Position;
    vec4 p1 = gl_in[1].gl_Position;
    vec4 p2 = gl_in[2].gl_Position;
    vec4 p3 = gl_in[3].gl_Position;

    // Interpolate along the bottom and top edges
    vec4 bottom = mix(p0, p1, gl_TessCoord.x);
    vec4 top = mix(p3, p2, gl_TessCoord.x);
    
    // Final position
    vec4 pos = mix(bottom, top, gl_TessCoord.y);

    // Interpolate UVs
    vec2 uv1 = mix(uv_tcs[0], uv_tcs[1], gl_TessCoord.x);
    vec2 uv2 = mix(uv_tcs[3], uv_tcs[2], gl_TessCoord.x);
    uv_tes = mix(uv1, uv2, gl_TessCoord.y);

    gl_Position = pos;
}
