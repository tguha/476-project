#ifndef ANIMATOR_H
#define ANIMATOR_H

#include <glm/glm.hpp>
#include <map>
#include <vector>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include "Animation.h"
#include "Bone.h"

class Animator
{
    public:
        Animator(Animation* animation);
        void UpdateAnimation(float dt);
        void PlayAnimation(Animation* panimation);
        void CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform);
        void SetCurrentAnimation(Animation* animation) { m_CurrentAnimation = animation; }
        Animation* GetCurrentAnimation() { return m_CurrentAnimation; }
        std::vector<glm::mat4> GetFinalBoneMatrices() { return m_FinalBoneMatrices; }
    private:
        std::vector<glm::mat4> m_FinalBoneMatrices;
        Animation* m_CurrentAnimation;
        float m_CurrentTime;
        float m_DeltaTime;
};

#endif // ANIMATOR_H