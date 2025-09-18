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
#include <Engine/Types.hpp>

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

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    int load = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    if (!load)
        return;
    glEnable(GL_DEPTH_TEST);
    stbi_set_flip_vertically_on_load(true);

    glfwSetWindowUserPointer(window, this);
    glfwSetKeyCallback(window, dispatchKeyCallback);
    glfwSetCursorPosCallback(window, dispatchMouseCallback);
}

GLFWwindow *OpenGLRenderer::get_window()
{
    return window;
}

int OpenGLRenderer::beginFrame()
{
    if (glfwWindowShouldClose(window))
    {
        destroy();
        return -1;
    }
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    return 0;
}

void OpenGLRenderer::endFrame()
{
    glfwSwapBuffers(window);
    glfwPollEvents();
}

float OpenGLRenderer::getTime()
{
    return glfwGetTime();
}

unsigned int OpenGLRenderer::loadShader(const char *vertexPath, const char *fragmentPath)
{
    auto shader = Shader(vertexPath, fragmentPath);
    return shader.ID;
}

void OpenGLRenderer::useShader(unsigned int shaderId)
{
    glUseProgram(shaderId);
    activeShader = shaderId;
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
    mesh->index_count = wMesh->indices.size();
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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
    glEnableVertexAttribArray(0);
    // vertex normals
    // glEnableVertexAttribArray(1);
    // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Normal));
    // vertex texture coords
    // glEnableVertexAttribArray(2);
    // glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, TexCoords));
    // vertex tangent
    // glEnableVertexAttribArray(3);
    // glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Tangent));
    // vertex bitangent
    // glEnableVertexAttribArray(4);
    // glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Bitangent));
    // ids
    // glEnableVertexAttribArray(5);
    // glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void *)offsetof(Vertex, m_BoneIDs));

    // weights
    // glEnableVertexAttribArray(6);
    // glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, m_Weights));
    // glBindVertexArray(0);

    meshes[wMesh->name] = mesh;
}

void OpenGLRenderer::renderMesh(WorldMesh *cmesh)
{
    uploadMesh(cmesh);
    auto iter = meshes.find(cmesh->name);
    assert(iter != meshes.end() && "Mesh not found, did you upload it?");
    auto mesh = iter->second;

    // bind appropriate textures
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;
    unsigned int normalNr = 1;
    unsigned int heightNr = 1;
    // for (unsigned int i = 0; i < cmesh->textures.size(); i++)
    // {
    //     glActiveTexture(GL_TEXTURE0 + i); // active proper texture unit before binding
    //     // retrieve texture number (the N in diffuse_textureN)
    //     std::string number;
    //     std::string name = cmesh->textures[i].type;
    //     if (name == "texture_diffuse")
    //         number = std::to_string(diffuseNr++);
    //     else if (name == "texture_specular")
    //         number = std::to_string(specularNr++); // transfer unsigned int to string
    //     else if (name == "texture_normal")
    //         number = std::to_string(normalNr++); // transfer unsigned int to string
    //     else if (name == "texture_height")
    //         number = std::to_string(heightNr++); // transfer unsigned int to string

    //     // now set the sampler to the correct texture unit
    //     glUniform1i(glGetUniformLocation(cmesh->shaderId, (name + number).c_str()), i);
    //     // and finally bind the texture
    //     // glBindTexture(GL_TEXTURE_2D, cmesh->textures[i].id);
    // }

    useShader(cmesh->shaderId);

    // draw mesh
    glBindVertexArray(mesh->VAO);
    glDrawElements(GL_TRIANGLES, mesh->index_count, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // always good practice to set everything back to defaults once configured.
    // glActiveTexture(GL_TEXTURE0);
}

void OpenGLRenderer::setViewMatrix(glm::mat4 view)
{
    unsigned int viewLoc = glGetUniformLocation(activeShader, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
}

void OpenGLRenderer::setProjectionMatrix(glm::mat4 projection)
{
    unsigned int projLoc = glGetUniformLocation(activeShader, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);
}

void OpenGLRenderer::setModelMatrix(glm::mat4 model)
{
    unsigned int modelLoc = glGetUniformLocation(activeShader, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
}

void OpenGLRenderer::dispatchKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    auto *renderer = static_cast<OpenGLRenderer *>(glfwGetWindowUserPointer(window));
    if (renderer)
        renderer->key_callback(window, key, scancode, action, mods);
}

KeyCode glfwKeyToEngineKey(int key)
{
    switch (key)
    {
    case GLFW_KEY_W:
        return W;
    case GLFW_KEY_A:
        return A;
    case GLFW_KEY_S:
        return S;
    case GLFW_KEY_D:
        return D;
    case GLFW_KEY_SPACE:
        return SPACE;
    case GLFW_KEY_LEFT_SHIFT:
        return LEFT_SHIFT;
    case GLFW_KEY_ESCAPE:
        return ESCAPE;
    default:
        return ESCAPE; // Default to ESCAPE for unknown keys
    }
}

KeyAction glfwActionToEngineAction(int action)
{
    switch (action)
    {
    case GLFW_PRESS:
        return PRESS;
    case GLFW_RELEASE:
        return RELEASE;
    case GLFW_REPEAT:
        return REPEAT;
    default:
        return RELEASE; // Default to RELEASE for unknown actions
    }
}

void OpenGLRenderer::key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    KeyCode keycode = glfwKeyToEngineKey(key);
    KeyAction keyaction = glfwActionToEngineAction(action);

    engineKeyCallback(keycode, keyaction);
}

void OpenGLRenderer::registerKeyCallback(std::function<void(KeyCode, KeyAction)> callback)
{
    engineKeyCallback = callback;
}

void OpenGLRenderer::mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    engineMouseCallback(xpos, ypos);
}

void OpenGLRenderer::registerMouseCallback(std::function<void(double, double)> callback)
{
    engineMouseCallback = callback;
}

void OpenGLRenderer::dispatchMouseCallback(GLFWwindow *window, double xpos, double ypos)
{
    auto *renderer = static_cast<OpenGLRenderer *>(glfwGetWindowUserPointer(window));
    if (renderer)
        renderer->mouse_callback(window, xpos, ypos);
}

void OpenGLRenderer::setMouseCapture(bool capture)
{
    if (capture)
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    else
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

bool OpenGLRenderer::initialized = false;