#version 410 core

// Interpolated values from the geometry shader.
in vec3 gsNormal;
in vec3 gsWorldPos;

// Final fragment color.
out vec4 color_out;

// Uniforms for lighting calculations.
uniform vec3 lightPos;    // Light position in world space.
uniform vec3 viewPos;     // Camera (eye) position in world space.
uniform vec4 objectColor; // Base color of the object.

void main()
{ 
    vec4 lightColor = vec4(1,1,1,1);

    // Normalize the normal interpolated from the geometry shader.
    vec3 normal = normalize(gsNormal);

    // Calculate the light direction vector.
    vec3 lightDir = normalize(lightPos - gsWorldPos);

    // Diffuse component using Lambert's cosine law.
    float diff = max(dot(normal, lightDir), 0.0);
    
    // Ambient component to simulate indirect light.
    float ambientStrength = 0.1;
    vec4 ambient = ambientStrength * lightColor;

    // Specular component for shiny highlights.
    vec4 viewDir = vec4(normalize(viewPos - gsWorldPos), 0.0f);
    vec4 reflectDir = vec4(reflect(-lightDir, normal), 0.0f);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    float specularStrength = 0.5;
    vec4 specular = specularStrength * spec * lightColor;

    // Combine lighting components with the object's base color.
    color_out = (ambient + diff * lightColor + specular) * objectColor;
}
