#version 330 core
out vec4 FragColor;
  
in vec2 TexCoord;

uniform sampler2D texture_diffuse0;

void main()
{
    if(texture(texture_diffuse0, TexCoord).a < 0.1)
        discard;
    FragColor = texture(texture_diffuse0, TexCoord);
}