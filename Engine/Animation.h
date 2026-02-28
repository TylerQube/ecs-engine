#include <string>
#include <glm/glm.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include<AssimpGLMHelpers.h>
#include <Bone.hAssimpNodeData>
#include <map>

struct AssimpNodeData {
    glm::mat4 transformation;
    std::string name;
    int childrenCount;
    std::vector<AssimpNodeData> children;
};

class Animation {
public:
    Animation() = default;

    Animation(const std::string& animationPath, Model* model) {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(animationPath, aiProcess_Triangulate);
        assert(scene && scene->mRootNode);

        auto anim = scene->mAnimations[0];
        duration = anim->mDuration;
        ticksPerSecond = anim->mTicksPerSecond;
        ReadHierarchyData(rootNode, scene->mRootNode);
        // ReadMissingBones(anim, *model); // implement if bones missing
    }

    ~Animation() {}

    Bone* FindBone(const std::string& name)
    {
        auto iter = std::find_if(m_Bones.begin(), m_Bones.end(),
            [&](const Bone& Bone)
            {
                return Bone.GetBoneName() == name;
            }
        );
        if (iter == m_Bones.end()) return nullptr;
        else return &(*iter);
    }

	
    inline float GetTicksPerSecond() { return ticksPerSecond; }

    inline float GetDuration() { return duration;}

    inline const AssimpNodeData& GetRootNode() { return rootNode; }

    inline const std::map<std::string,BoneInfo>& GetBoneIDMap() 
    { 
        return m_BoneInfoMap;
    }

private:
    AssimpNodeData rootNode;
    float duration;
    int ticksPerSecond;

    void ReadHierarchyData(AssimpNodeData& dst, const aiNode* src) {
        assert(src);

        dst.name = src->mName.data;
        dst.transformation = AssimpGLMHelpers::ConvertMatrixToGLMFormat(src->mTransformation);
        dst.childrenCount = src->mNumChildren;

        for(int i = 0; i < src->mNumChildren; i++) {
            AssimpNodeData newData;
            ReadHierarchyData(newData, src->mChildren[i]);
            dst.children.push_back(newData);
        }
    }
}