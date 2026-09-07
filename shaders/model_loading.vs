#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 bitangent;
layout(location = 5) in ivec4 boneIds; 
layout(location = 6) in vec4 weights;

out vec2 TexCoords;
out vec3 Normal;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 lightPos;
uniform int useSkinning;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[100];

void main()
{
    vec4 localPosition = vec4(aPos, 1.0f);
    vec3 localNormal = aNormal;

    if (useSkinning != 0) {
        vec4 totalPosition = vec4(0.0f);
        vec3 totalNormal = vec3(0.0f);
        for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)
        {
            if(boneIds[i] == -1) 
                continue;
            if(boneIds[i] >= MAX_BONES) 
                continue;
            vec4 skinnedPosition = finalBonesMatrices[boneIds[i]] * vec4(aPos, 1.0f);
            vec3 skinnedNormal = mat3(finalBonesMatrices[boneIds[i]]) * aNormal;
            totalPosition += skinnedPosition * weights[i];
            totalNormal += skinnedNormal * weights[i];
        }
        localPosition = totalPosition;
        localNormal = totalNormal;
    }

    vec4 worldPosition = model * localPosition;
    float snapSize = 0.02;
    worldPosition.xyz = floor(worldPosition.xyz / snapSize + 0.5) * snapSize;

    gl_Position = projection * view * worldPosition;
	TexCoords = aTexCoords;

    FragPos = worldPosition.xyz;
    Normal = normalize(mat3(transpose(inverse(model))) * normalize(localNormal));
}