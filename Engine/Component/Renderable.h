#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>

#include <string>
#include <vector>

#define MAX_BONE_INFLUENCE 4

using MeshId = unsigned int;

struct Vertex
{
    glm::vec3 Position;
    // normal
    glm::vec3 Normal;
    // texCoords
    glm::vec2 TexCoords;
};

struct Texture
{
    unsigned int id;
    std::string type;
    std::string path;
};

struct WorldMesh
{
    unsigned int shaderId;
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    std::string name;
};

struct Renderable
{
    std::vector<WorldMesh> meshes;
};