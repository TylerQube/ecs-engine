#include "Renderer/Renderer.h"
#include "OpenGLRenderer.h"
#include "Shader.h"
#include <cassert>
#include <iostream>
#include <memory>

#include "glad/glad.h"
#include "glfw/glfw3.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

void OpenGLRenderer::init()
{
    if (initialized)
        return;

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    stbi_set_flip_vertically_on_load(true);

    initialized = true;
}

void OpenGLRenderer::destroy()
{
    glfwTerminate();
}

void OpenGLRenderer::framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

OpenGLRenderer::OpenGLRenderer(const char *title, unsigned int width, unsigned int height)
{
    if (!initialized)
        init();
    window =
        glfwCreateWindow(width, height, title, NULL, NULL);
    assert(window && "Failed to create GLFW window");

    WIDTH = width;
    HEIGHT = height;

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glfwMakeContextCurrent(window);

    int load = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    if (!load)
        return;
    glEnable(GL_DEPTH_TEST);
    stbi_set_flip_vertically_on_load(true);
}

GLFWwindow *OpenGLRenderer::get_window()
{
    return window;
}

void OpenGLRenderer::run()
{
    std::cout << "Running render loop" << std::endl;
    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    destroy();
}

unsigned int loadShader(const char *vertexPath, const char *fragmentPath)
{
    auto shader = Shader(vertexPath, fragmentPath);
    return shader.ID;
}

void useShader(unsigned int shaderId)
{
    glUseProgram(shaderId);
}

void OpenGLRenderer::uploadMesh(WorldMesh *wMesh)
{
    // auto iter = meshes.find(wMesh->getType());
    // if (iter != meshes.end())
    // {
    //     // already loaded
    //     return;
    // }

    auto mesh = std::make_shared<RenderMesh>();
    // create buffers/arrays
    glGenVertexArrays(1, &mesh->VAO);
    glGenBuffers(1, &mesh->VBO);
    glGenBuffers(1, &mesh->EBO);

    glBindVertexArray(mesh->VAO);
    // load data into vertex buffers
    glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO);
    glBufferData(GL_ARRAY_BUFFER, wMesh->vertices.size() * sizeof(Vertex), &wMesh->vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, wMesh->indices.size() * sizeof(unsigned int), &wMesh->indices[0], GL_STATIC_DRAW);

    // set the vertex attribute pointers
    // vertex Positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, TexCoords));
    // vertex tangent
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Tangent));
    // vertex bitangent
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Bitangent));
    // ids
    glEnableVertexAttribArray(5);
    glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void *)offsetof(Vertex, m_BoneIDs));

    // weights
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, m_Weights));
    glBindVertexArray(0);

    meshes[wMesh->name] = mesh;
}

void OpenGLRenderer::renderMesh(WorldMesh *cmesh)
{
    auto iter = meshes.find(cmesh->name);
    assert(iter != meshes.end() && "Mesh not found, did you upload it?");
    auto mesh = iter->second;

    // bind appropriate textures
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;
    unsigned int normalNr = 1;
    unsigned int heightNr = 1;
    for (unsigned int i = 0; i < cmesh->textures.size(); i++)
    {
        glActiveTexture(GL_TEXTURE0 + i); // active proper texture unit before binding
        // retrieve texture number (the N in diffuse_textureN)
        std::string number;
        std::string name = cmesh->textures[i].type;
        if (name == "texture_diffuse")
            number = std::to_string(diffuseNr++);
        else if (name == "texture_specular")
            number = std::to_string(specularNr++); // transfer unsigned int to string
        else if (name == "texture_normal")
            number = std::to_string(normalNr++); // transfer unsigned int to string
        else if (name == "texture_height")
            number = std::to_string(heightNr++); // transfer unsigned int to string

        // now set the sampler to the correct texture unit
        glUniform1i(glGetUniformLocation(cmesh->shaderId, (name + number).c_str()), i);
        // and finally bind the texture
        glBindTexture(GL_TEXTURE_2D, cmesh->textures[i].id);
    }

    // draw mesh
    glBindVertexArray(mesh->VAO);
    glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(cmesh->indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // always good practice to set everything back to defaults once configured.
    glActiveTexture(GL_TEXTURE0);
}

bool OpenGLRenderer::initialized = false;