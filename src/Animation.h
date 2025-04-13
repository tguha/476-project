#ifndef ANIMATION_H
#define ANIMATION_H

#include <vector>
#include <map>
#include <glm/glm.hpp>
#include <assimp/scene.h>
#include <iostream>
#include "Bone.h"
#include <functional>
#include "AssimpModel.h"

struct AssimpNodeData
{
    glm::mat4 transformation;
    std::string name;
    int childrenCount;
    std::vector<AssimpNodeData> children;
};

class Animation
{
    public:
        Animation() = default;

        Animation(const std::string& animationPath, AssimpModel* model, int animationIndex);
        ~Animation();
        Bone* FindBone(const std::string& name);
        inline float GetTicksPerSecond() { return m_TicksPerSecond; }
        inline float GetDuration() { return m_Duration; }
        inline const AssimpNodeData& GetRootNode() { return m_RootNode; }
        inline const std::map<std::string, BoneInfo>& GetBoneIDMap() { return m_BoneInfoMap; }

        // void setAnimation(int animIndex, AssimpModel* model);
    private:
        void ReadMissingBones(const aiAnimation* animation, AssimpModel& model);
        void ReadHierarchyData(AssimpNodeData& dest, const aiNode* src);
        float m_Duration;
        int m_TicksPerSecond;
        std::vector<Bone*> m_Bones;
        AssimpNodeData m_RootNode;
        std::map<std::string, BoneInfo> m_BoneInfoMap;

        // struct AnimationData
        // {
        //     std::string name;
        //     float duration;
        //     float ticksPerSecond;
        // };
        // std::vector<AnimationData> m_AnimationData;
        // const aiScene* m_Scene;
};

#endif // ANIMATION_H