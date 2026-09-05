#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 bitangent;
layout(location = 5) in ivec4 boneIds; 
layout(location = 6) in vec4 weights;

out vec2 TexCoords;
out vec3 LightPos;
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

    if (useSkinning != 0) {
        vec4 totalPosition = vec4(0.0f);
        for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)
        {
            if(boneIds[i] == -1) 
                continue;
            if(boneIds[i] >= MAX_BONES) 
                continue;
            vec4 skinnedPosition = finalBonesMatrices[boneIds[i]] * vec4(aPos, 1.0f);
            totalPosition += skinnedPosition * weights[i];
        }
        localPosition = totalPosition;
    }

    vec4 worldPosition = model * localPosition;
    float snapSize = 0.01;
    worldPosition.xyz = floor(worldPosition.xyz / snapSize + 0.5) * snapSize;

    gl_Position = projection * view * worldPosition;
	TexCoords = aTexCoords;

    FragPos = vec3(view * model * vec4(aPos, 1.0f));
    Normal = mat3(transpose(inverse(model))) * aNormal;  
    LightPos = vec3(view * vec4(lightPos, 1.0f));
}