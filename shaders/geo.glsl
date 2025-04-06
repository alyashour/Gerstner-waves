#version 410 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

// Input from tess eval shader
in vec2 uv_tes[];

// Output to fragment shader
out vec3 gsNormal;
out vec3 gsWorldPos;

uniform mat4 MVP;
uniform sampler2D distext;
uniform float time;

// Calculate a triangle’s normal from three positions.
vec3 GetNormal(vec4 a, vec4 b, vec4 c)
{
    vec3 x = b.xyz - a.xyz;
    vec3 y = c.xyz - a.xyz;
    return normalize(cross(x, y));
}

// Gerstner wave function.
// For a given world position, computes a displacement due to one wave.
// Parameters:
// - worldpos: the input world coordinate.
// - w: wave frequency.
// - A: amplitude.
// - phi: phase multiplier.
// - Q: sharpness factor (between 0 and 1).
// - D: 2D direction vector for the wave.
// - N: exponent controlling the influence of Q.
vec3 Gerstner(vec3 worldpos, float w, float A, float phi, float Q, vec2 D, int N)
{
    // Compute the effective sharpness value: Qi = w * A * (Q^N)
    float Qi = w * A * pow(Q, float(N));
    // Dot product between the wave direction and the (x,z) components.
    float dotProduct = dot(D, worldpos.xz);
    // Compute phase: frequency * dot + phase shift scaled by time.
    float phase = w * dotProduct + phi * time;
    // Displacements in x and z are scaled by Qi and the respective D components.
    float dx = D.x * A * cos(phase) * Qi;
    float dz = D.y * A * cos(phase) * Qi;
    // Vertical (y) displacement is given by the sine of the phase.
    float dy = A * sin(phase);
    return vec3(dx, dy, dz);
}

void main()
{
    vec4 pos[3];

    // Process vertices
    for (int i = 0; i < 3; ++i)
    {
        pos[i] = gl_in[i].gl_Position;
        
        // Use the passed UVs to sample displacement
        float disp = texture(distext, uv_tes[i]).r;
        pos[i].y += disp;

        // Add Gerstner waves
        pos[i] += vec4(Gerstner(pos[i].xyz, 4.0, 0.08, 1.1, 0.75, vec2(0.3, 0.6), 4), 0.0);
        pos[i] += vec4(Gerstner(pos[i].xyz, 2.0, 0.05, 1.1, 0.75, vec2(0.2, 0.866), 4), 0.0);
        pos[i] += vec4(Gerstner(pos[i].xyz, 0.6, 0.2, 0.4, 0.1, vec2(0.3, 0.7), 4), 0.0);
        pos[i] += vec4(Gerstner(pos[i].xyz, 0.9, 0.15, 0.4, 0.1, vec2(0.8, 0.1), 4), 0.0);
    }

    // Calculate normal for the triangle
    vec3 normal = GetNormal(pos[0], pos[1], pos[2]);

    // Emit vertices
    for (int i = 0; i < 3; ++i)
    {
        gsWorldPos = pos[i].xyz;
        gsNormal = normal;
        gl_Position = MVP * pos[i];
        EmitVertex();
    }
    EndPrimitive();
}
