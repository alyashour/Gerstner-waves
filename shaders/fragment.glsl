#version 410 core

// Interpolated values from the geometry shader.
in vec3 gsNormal;
in vec3 gsWorldPos;

// Final fragment color.
out vec4 color_out;

// Uniforms for lighting calculations.
uniform vec3 lightPos;    
uniform vec3 viewPos;     
uniform vec4 objectColor; 
uniform sampler2D waterTexture;

void main()
{ 
    // Brighter light color
    vec4 LightColor = vec4(1.2, 1.2, 1.2, 1.0);

    vec2 waterUV = gsWorldPos.xz * 0.1;
    // Boost the water texture colors
    vec4 MaterialDiffuseColor = texture(waterTexture, waterUV) * vec4(1.2, 1.2, 1.4, 1.0);
    // Brighter ambient color
    vec4 MaterialAmbientColor = vec4(0.4, 0.4, 0.5, 1.0) * MaterialDiffuseColor;
    // Increased specular intensity
    vec4 MaterialSpecularColor = vec4(0.9, 0.9, 1.0, 1.0);

    // Normalize vectors
    vec3 normal = normalize(gsNormal);
    vec3 lightDir = normalize(lightPos - gsWorldPos);
    vec3 viewDir = normalize(viewPos - gsWorldPos);
    vec3 reflectDir = reflect(-lightDir, normal);

    // Calculate cosine terms
    float cosTheta = max(dot(normal, lightDir), 0.0);
    float cosAlpha = max(dot(viewDir, reflectDir), 0.0);
    
    // Increased ambient strength
    float ambientStrength = 0.5;
    vec4 ambient = ambientStrength * MaterialDiffuseColor;

    // Final color calculation with boosted intensity
    color_out = 
        MaterialAmbientColor * 1.2 +
        MaterialDiffuseColor * LightColor * cosTheta * 1.5 +
        MaterialSpecularColor * LightColor * pow(cosAlpha, 16) * 1.3;
        
    // Optional: Add a slight blue tint to the final color
    color_out = mix(color_out, vec4(0.2, 0.4, 0.8, 1.0), 0.2);
}
