#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 TexCoord;
out vec3 VertColor;

struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
};

uniform int numPointLights;
uniform PointLight pointLights[32];
uniform vec3 viewPos;
uniform int material_receivesLight;

void main() {
    vec4 worldPos = model * vec4(aPos, 1.0);
    vec3 FragPos = worldPos.xyz;
    vec3 N = normalize(mat3(transpose(inverse(model))) * aNormal);

    if (material_receivesLight == 0 || numPointLights == 0) {
        VertColor = vec3(1.0);
    } else {
        vec3 result = vec3(0.0);
        vec3 albedo = vec3(1.0); // texture will modulate this in fragment
        for (int i = 0; i < numPointLights; ++i) {
            vec3 lightDir = normalize(pointLights[i].position - FragPos);
            vec3 lightFacingNormal = faceforward(N, -lightDir, N);
            float distance = length(pointLights[i].position - FragPos);
            float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);

            float diff = max(dot(lightFacingNormal, lightDir), 0.0);
            vec3 viewDir = normalize(viewPos - FragPos);
            vec3 halfwayDir = normalize(lightDir + viewDir);
            float spec = pow(max(dot(lightFacingNormal, halfwayDir), 0.0), 32.0);

            vec3 ambient = 0.1 * albedo;
            vec3 diffuse = diff * albedo * pointLights[i].color * pointLights[i].intensity;
            vec3 specular = spec * vec3(0.3) * pointLights[i].color * pointLights[i].intensity;

            result += (ambient + diffuse + specular) * attenuation;
        }
        VertColor = result;
    }

    TexCoord = aTexCoord;
    gl_Position = projection * view * worldPos;
}
