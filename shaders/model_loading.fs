#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;
in vec3 LightPos;

uniform sampler2D texture_diffuse1;

void main()
{    
    // FragColor = texture(texture_diffuse1, TexCoords);
    FragColor = vec4(1.0, 0.0, 0.0, 1.0); // Set to red for testing
}