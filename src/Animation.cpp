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
    m_GlobalInverseTransform = AssimpGLMHelpers::ConvertMatrixToGLMFormat(globalTransformation);
    ReadHierarchyData(m_RootNode, scene->mRootNode);
    ReadMissingBones(animation, *model);
    
    if (verbose_debug) {
        std::cout << "Root transform (bind pose):\n";
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                std::cout << scene->mRootNode->mTransformation[j][i] << " | ";
            }
            std::cout << std::endl;
        }
        std::cout << "Transformation Inverse (bind pose):\n";
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                std::cout << globalTransformation[j][i] << " | ";
            }
            std::cout << std::endl;
        }

    }
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

    //Testing For Bone Offset Matricies
    if (verbose_debug){
        for (const auto& pair : boneInfoMap) {
            std::cout << pair.first << " offset:\n";
            for (int i = 0; i < 4; ++i) {
                for (int j = 0; j < 4; ++j) {
                    std::cout << pair.second.offset[j][i] << " | ";

                }
                std::cout << std::endl;
            }
        }
    }


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

