#include "Animation.h"
// #include <assimp/Importer.hpp>

Animation::Animation(const std::string& animationPath, AssimpModel* model, int animationIndex) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(animationPath, aiProcess_Triangulate);
    assert(scene && scene->mRootNode);
    auto animation = scene->mAnimations[animationIndex];
    if (animationIndex < 0 || animationIndex >= scene->mNumAnimations) {
        std::cout << "Invalid animation index: " << animationIndex << std::endl;
        return;
    }
    std::cout << "Animation name: " << animation->mName.C_Str() << std::endl;
    m_Duration = animation->mDuration;
    m_TicksPerSecond = animation->mTicksPerSecond;
    aiMatrix4x4 globalTransformation = scene->mRootNode->mTransformation;
    globalTransformation = globalTransformation.Inverse();
    ReadHierarchyData(m_RootNode, scene->mRootNode);
    ReadMissingBones(animation, *model);
    // m_Scene = importer.ReadFile(animationPath, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType);
    // assert(m_Scene && m_Scene->mRootNode);
    // aiMatrix4x4 globalTransformation = m_Scene->mRootNode->mTransformation;
    // globalTransformation = globalTransformation.Inverse();
    // ReadHierarchyData(m_RootNode, m_Scene->mRootNode);
    // for (unsigned int i = 0; i < m_Scene->mNumAnimations; i++) {
    //     aiAnimation* animation = m_Scene->mAnimations[i];
    //     AnimationData animData;
    //     animData.name = animation->mName.data ? animation->mName.data : "Unnamed";
    //     animData.duration = animation->mDuration;
    //     animData.ticksPerSecond = animation->mTicksPerSecond ? animation->mTicksPerSecond : 25.0f; // Default value if not specified

    //     std::cout << "Found animation " << i << ": "
    //         << animation->mName.C_Str()
    //         << " duration: " << animation->mDuration
    //         << " ticks per second: " << animation->mTicksPerSecond << std::endl;

    //     m_AnimationData.push_back(animData);
    // }
}

Animation::~Animation() {
}

Bone* Animation::FindBone(const std::string& name) {
    auto iter = std::find_if(m_Bones.begin(), m_Bones.end(), [&](const Bone* bone) {
        return bone->GetBoneName() == name;
    });
    if (iter == m_Bones.end()) {
        return nullptr;
    } else {
        return *iter;
    }
}

void Animation::ReadMissingBones(const aiAnimation* animation, AssimpModel& model) {
    int size = animation->mNumChannels;

    auto& boneInfoMap = model.GetBoneInfoMap();
    int& boneCount = model.GetBoneCounter();

    // reading channels
    for (int i = 0; i < size; i++) {
        auto channel = animation->mChannels[i];
        std::string boneName = channel->mNodeName.data;
        if (boneInfoMap.find(boneName) == boneInfoMap.end()) {
            boneInfoMap[boneName].id = boneCount;
            boneCount++;
        }
        m_Bones.push_back(new Bone(channel->mNodeName.data, boneInfoMap[channel->mNodeName.data].id, channel));
    }

    m_BoneInfoMap = boneInfoMap;
}

void Animation::ReadHierarchyData(AssimpNodeData& dest, const aiNode* src) {
    assert(src);

    dest.name = src->mName.data;
    dest.transformation = AssimpGLMHelpers::ConvertMatrixToGLMFormat(src->mTransformation);
    dest.childrenCount = src->mNumChildren;

    for (int i = 0; i < src->mNumChildren; i++) {
        AssimpNodeData newData;
        ReadHierarchyData(newData, src->mChildren[i]);
        dest.children.push_back(newData);
    }
}

// void Animation::setAnimation(int animIndex, AssimpModel* model) {
//     if (animIndex < 0 || animIndex >= m_AnimationData.size()) {
//         std::cout << "Invalid animation index: " << animIndex << std::endl;
//         return;
//     }
//     m_Duration = m_AnimationData[animIndex].duration;
//     m_TicksPerSecond = m_AnimationData[animIndex].ticksPerSecond;

//     aiAnimation* animation = m_Scene->mAnimations[animIndex];

//     m_Bones.clear();
//     std::cout << "Animation name: " << animation->mName.C_Str() << std::endl;
//     ReadMissingBones(animation, *model);

// }

