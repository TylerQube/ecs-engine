#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 VertColor;

uniform sampler2D texture_diffuse0;

void main()
{
    vec4 tex = texture(texture_diffuse0, TexCoord);
    if (tex.a < 0.1) discard;

    vec3 color = tex.rgb * VertColor;
    FragColor = vec4(color, tex.a);
}
