#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <vector>
#include <string>
#include <assimp/anim.h>
#include <AssimpGLMHelpers.h>

/**
 * Referenced from LearnOpenGL Skeletal Animation Article:
 * https://learnopengl.com/Guest-Articles/2020/Skeletal-Animation
 */

struct BoneInfo {
    int id;
    glm::mat4 offset; // model to bone-space
};

struct KeyPosition
{
    glm::vec3 position;
    float timestamp;
};

struct KeyRotation
{
    glm::quat orientation;
    float timestamp;
};

struct KeyScale
{
    glm::vec3 scale;
    float timestamp;
};

class Bone
{
private:
    std::vector<KeyPosition> positions;
    std::vector<KeyRotation> rotations;
    std::vector<KeyScale> scales;
    int numPositions;
    int numRotations;
    int numScales;

    glm::mat4 localTransform;
    std::string name;
    int id;

public:

    std::string GetBoneName() const { return name; }
    // read in keyframes from assimp
    Bone(const std::string &name, int ID, const aiNodeAnim *channel)
        : name(name), id(ID), localTransform(1.0f)
    {
        numPositions = channel->mNumPositionKeys;
        for (int i = 0; i < numPositions; i++)
        {
            aiVector3D aiPos = channel->mPositionKeys[i].mValue;
            float timestamp = channel->mPositionKeys[i].mTime;
            KeyPosition data;
            data.position = AssimpGLMHelpers::GetGLMVec(aiPos);
            data.timestamp = timestamp;
            positions.push_back(data);
        }

        numRotations = channel->mNumRotationKeys;
        for (int i = 0; i < numRotations; i++)
        {
            aiQuaternion aiOrientation = channel->mRotationKeys[i].mValue;
            float timestamp = channel->mRotationKeys[i].mTime;
            KeyRotation data;
            data.orientation = AssimpGLMHelpers::GetGLMQuat(aiOrientation);
            data.timestamp = timestamp;
            rotations.push_back(data);
        }

        numScales = channel->mNumScalingKeys;
        for (int i = 0; i < numScales; ++i)
        {
            aiVector3D scale = channel->mScalingKeys[i].mValue;
            float timestamp = channel->mScalingKeys[i].mTime;
            KeyScale data;
            data.scale = AssimpGLMHelpers::GetGLMVec(scale);
            data.timestamp = timestamp;
            scales.push_back(data);
        }
    }

    void Update(float animTime) {
        glm::mat4 translation = InterpolatePosition(animTime);
        glm::mat4 rotation = InterpolateRotation(animTime);
        glm::mat4 scale = InterpolateScale(animTime);
        localTransform = translation * rotation * scale;
    }

    glm::mat4 GetLocalTransform() { return localTransform; }
    std::string GetBonName() const { return name; }
    int GetBoneId() { return id; }

    int GetPositionIndex(float animTime) {
        for (int index = 0; index < numPositions - 1; ++index)
        {
            if (animTime < positions[index + 1].timestamp)
                return index;
        }
        assert(0);
    }

    int GetRotationIndex(float animTime) {
        for (int index = 0; index < numRotations - 1; ++index)
        {
            if (animTime < rotations[index + 1].timestamp)
                return index;
        }
        assert(0);
    }


    int GetScaleIndex(float animTime) {
        for (int index = 0; index < numScales - 1; ++index)
        {
            if (animTime < scales[index + 1].timestamp)
                return index;
        }
        assert(0);
    }

private:
    /* Gets normalized value for Lerp & Slerp*/
    float GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime)
    {
        float scaleFactor = 0.0f;
        float midWayLength = animationTime - lastTimeStamp;
        float framesDiff = nextTimeStamp - lastTimeStamp;
        scaleFactor = midWayLength / framesDiff;
        return scaleFactor;
    }

    /*figures out which position keys to interpolate b/w and performs the interpolation 
    and returns the translation matrix*/
    glm::mat4 InterpolatePosition(float animationTime)
    {
        if (1 == numPositions)
            return glm::translate(glm::mat4(1.0f), positions[0].position);

        int p0Index = GetPositionIndex(animationTime);
        int p1Index = p0Index + 1;
        float scaleFactor = GetScaleFactor(positions[p0Index].timestamp,
            positions[p1Index].timestamp, animationTime);
        glm::vec3 finalPosition = glm::mix(positions[p0Index].position,
            positions[p1Index].position, scaleFactor);
        return glm::translate(glm::mat4(1.0f), finalPosition);
    }

    /*figures out which rotations keys to interpolate b/w and performs the interpolation 
    and returns the rotation matrix*/
    glm::mat4 InterpolateRotation(float animationTime)
    {
        if (1 == numRotations)
        {
            auto rotation = glm::normalize(rotations[0].orientation);
            return glm::toMat4(rotation);
        }

        int p0Index = GetRotationIndex(animationTime);
        int p1Index = p0Index + 1;
        float scaleFactor = GetScaleFactor(rotations[p0Index].timestamp,
            rotations[p1Index].timestamp, animationTime);
        glm::quat finalRotation = glm::slerp(rotations[p0Index].orientation,
            rotations[p1Index].orientation, scaleFactor);
        finalRotation = glm::normalize(finalRotation);
        return glm::toMat4(finalRotation);
    }

    /*figures out which scaling keys to interpolate b/w and performs the interpolation 
    and returns the scale matrix*/
    glm::mat4 Bone::InterpolateScale(float animationTime)
    {
        if (1 == numScales)
            return glm::scale(glm::mat4(1.0f), scales[0].scale);

        int p0Index = GetScaleIndex(animationTime);
        int p1Index = p0Index + 1;
        float scaleFactor = GetScaleFactor(scales[p0Index].timestamp,
            scales[p1Index].timestamp, animationTime);
        glm::vec3 finalScale = glm::mix(scales[p0Index].scale, scales[p1Index].scale
            , scaleFactor);
        return glm::scale(glm::mat4(1.0f), finalScale);
    }
};