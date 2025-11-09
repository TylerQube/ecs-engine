#pragma once
#include "Shader.h"

class Material
{
public:
    Shader *m_shader;

    Material(const char* vertexSource, const char* fragmentSource)
    {
        m_shader = new Shader(vertexSource, fragmentSource);
    }

    bool set(const std::string uniform)
    {
        auto it = m_shader->uniformInfoMap.find(uniform);
        if(it == m_shader->uniformInfoMap.end()) {
            std::cerr << "Uniform not found: " << uniform << std::endl;
            return false;
        }
        return true;
    }

    int set(const std::string uniform, unsigned int value) {
        if(!set(uniform)) return -1;
        m_shader->setInt(uniform, value);
        return 0;
    }

    int set(const std::string uniform, float value) {
        if(!set(uniform)) return -1;
        m_shader->setFloat(uniform, value);
        return 0;
    }

    int set(const std::string uniform, glm::vec3 value) {
        if(!set(uniform)) return -1;
        m_shader->setVec3(uniform, value);
        return 0;
    }

    int set(const std::string uniform, glm::vec4 value) {
        if(!set(uniform)) return -1;
        m_shader->set4Float(uniform, value.x, value.y, value.z, value.w);
        return 0;
    }

    int set(const std::string uniform, glm::mat4 value) {
        if(!set(uniform)) return -1;
        m_shader->use();
        m_shader->setMat4(uniform, value);
        return 0;
    }
};