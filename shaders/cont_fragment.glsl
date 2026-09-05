#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D texture_diffuse0;

struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
};

uniform int numPointLights;
uniform PointLight pointLights[32];
uniform vec3 viewPos;
uniform int material_receivesLight; // 0 or 1

void main()
{
    vec4 tex = texture(texture_diffuse0, TexCoord);
    if(tex.a < 0.1)
        discard;

    if(material_receivesLight == 0 || numPointLights == 0) {
        FragColor = tex;
        return;
    }

    vec3 N = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 result = vec3(0.0);
    vec3 albedo = tex.rgb;

    for(int i = 0; i < numPointLights; ++i) {
        vec3 lightDir = normalize(pointLights[i].position - FragPos);
        float distance = length(pointLights[i].position - FragPos);
        float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * (distance * distance));

        // diffuse
        float diff = max(dot(N, lightDir), 0.0);

        // specular (Blinn-Phong)
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(N, halfwayDir), 0.0), 32.0);

        vec3 ambient = 0.1 * albedo;
        vec3 diffuse = diff * albedo * pointLights[i].color * pointLights[i].intensity;
        vec3 specular = spec * vec3(0.3) * pointLights[i].color * pointLights[i].intensity;

        result += (ambient + diffuse + specular) * attenuation;
    }

    FragColor = vec4(result, tex.a);
}