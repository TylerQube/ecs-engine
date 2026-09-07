#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>

#include <string>
#include <vector>
#include <map>
#include <Engine/Animation/Bone.h>

#define MAX_BONE_INFLUENCE 4

using MeshId = unsigned int;

struct Vertex
{
    glm::vec3 Position;
    // normal
    glm::vec3 Normal;
    // texCoords
    glm::vec2 TexCoords;

    glm::vec3 Tangent;
    glm::vec3 Bitangent;

	//bone indexes which will influence this vertex
	int m_BoneIDs[MAX_BONE_INFLUENCE];
	//weights from each bone
	float m_Weights[MAX_BONE_INFLUENCE];
};

struct Texture
{
    unsigned int id;
    std::string type;
    std::string path;
};

struct Material {
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float shininess;
    std::vector<Texture> textures;

    bool receivesLight = true;
};

struct WorldMesh
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    Material material;

    std::string name;
};


struct Model
{
    unsigned int shaderId;
    std::vector<WorldMesh> meshes;
    bool hasLocalBounds = false;
    glm::vec3 localBoundsMin = glm::vec3(0.0f);
    glm::vec3 localBoundsMax = glm::vec3(0.0f);
    bool visibleInFrustum = true;

    std::map<std::string, BoneInfo> boneInfo;
    int boneCount;
};