#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>

#include <string>
#include <vector>
#include <Bone.h>

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

struct WorldMesh
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    std::string name;
};


struct Renderable
{
    unsigned int shaderId;
    std::vector<WorldMesh> meshes;
};

struct Model
{
    unsigned int shaderId;
    std::vector<WorldMesh> meshes;
    std::vector<Texture> textures;
    std::string directory;

    std::map<std::string, BoneInfo> boneInfo;
    int boneCount;
};
