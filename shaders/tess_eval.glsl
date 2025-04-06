#version 410 core

// Specify that the input patch is a quad, using equal spacing and clockwise winding.
layout(quads, equal_spacing, cw) in;

uniform mat4 MVP;

void main() {
    // For a quad patch, we assume the control points are ordered as:
    // p0: bottom-left, p1: bottom-right, p2: top-right, p3: top-left.
    vec4 p0 = gl_in[0].gl_Position;
    vec4 p1 = gl_in[1].gl_Position;
    vec4 p2 = gl_in[2].gl_Position;
    vec4 p3 = gl_in[3].gl_Position;

    // Interpolate along the bottom and top edges.
    vec4 bottom = mix(p0, p1, gl_TessCoord.x);
    vec4 top = mix(p3, p2, gl_TessCoord.x);

    // Interpolate between the bottom and top edges.
    vec4 pos = mix(bottom, top, gl_TessCoord.y);

    // Apply the MVP transformation.
    gl_Position = MVP * pos;
}
