#version 330 core
out vec4 FragColor;
  
in vec2 TexCoord;

uniform sampler2D myTexture;
uniform vec4 color;

void main()
{
    if(texture(myTexture, TexCoord).a < 0.1)
        discard;
    FragColor = texture(myTexture, TexCoord) * color;
}