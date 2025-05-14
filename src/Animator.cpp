#include "Animator.h"
#include <iostream>

Animator::Animator(Animation* animation)
{
    m_CurrentTime = 0.0;
    m_CurrentAnimation = animation;

    m_FinalBoneMatrices.reserve(100);

    for (int i = 0; i < 100; i++)
    {
        m_FinalBoneMatrices.push_back(glm::mat4(1.0f));
    }
}

void Animator::UpdateAnimation(float dt)
{
    float tickRate = m_CurrentAnimation->GetTicksPerSecond();
    if (tickRate <= 0) {
        tickRate = 25.0f; // Default value if not specified
    }
    m_CurrentTime += dt * tickRate; // Update current time based on delta time and ticks per second
    m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->GetDuration()); // Loop the animation
    CalculateBoneTransform(&m_CurrentAnimation->GetRootNode(), m_CurrentAnimation->GetGlobalInverseTransform());
    
}

void Animator::PlayAnimation(Animation* pAnimation) {
    m_CurrentAnimation = pAnimation;
    m_CurrentTime = 0.0;
}

void Animator::CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform)
{
    std::string nodeName = node->name;
    glm::mat4 nodeTransform = node->transformation;

    Bone* Bone = m_CurrentAnimation->FindBone(nodeName);

    if (Bone)
    {
        Bone->Update(m_CurrentTime);
        nodeTransform = Bone->GetLocalTransform();
    }

    glm::mat4 globalTransformation = parentTransform * nodeTransform ;

    auto boneInfoMap = m_CurrentAnimation->GetBoneIDMap();
    if (boneInfoMap.find(nodeName) != boneInfoMap.end())
    {
        int index = boneInfoMap[nodeName].id;
        glm::mat4 offset = boneInfoMap[nodeName].offset;
        m_FinalBoneMatrices[index] = globalTransformation * offset;
        // m_FinalBoneMatrices[index] = offset * globalTransformation; // for fbx
    }

    for (int i = 0; i < node->childrenCount; i++)
    {
        CalculateBoneTransform(&node->children[i], globalTransformation);
    }

}