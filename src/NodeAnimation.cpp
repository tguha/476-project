#include "NodeAnimation.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <iostream>

NodeAnimation::NodeAnimation(const aiScene* scene, int animIndex)
    : duration(scene->mAnimations[animIndex]->mDuration),
      ticksPerSecond(scene->mAnimations[animIndex]->mTicksPerSecond ? scene->mAnimations[animIndex]->mTicksPerSecond : 25.0f)
{
    const aiAnimation* animation = scene->mAnimations[animIndex];
    for (unsigned int i = 0; i < animation->mNumChannels; ++i) {
        const aiNodeAnim* nodeAnim = animation->mChannels[i];
        nodeAnims[nodeAnim->mNodeName.C_Str()] = nodeAnim;
    }
}

const aiNodeAnim* NodeAnimation::GetNodeAnimation(const std::string& nodeName) const {
    auto it = nodeAnims.find(nodeName);
    return it != nodeAnims.end() ? it->second : nullptr;
}

glm::mat4 NodeAnimation::InterpolateNode(const std::string& nodeName, float time) const {
    const aiNodeAnim* channel = GetNodeAnimation(nodeName);
    if (!channel) return glm::mat4(1.0f);

    glm::vec3 pos = InterpolatePosition(channel, time);
    glm::quat rot = InterpolateRotation(channel, time);
    glm::vec3 scale = InterpolateScaling(channel, time);

    return glm::translate(glm::mat4(1.0f), pos) * glm::toMat4(rot) * glm::scale(glm::mat4(1.0f), scale);
}

// --- Interpolation logic ---
glm::vec3 NodeAnimation::InterpolatePosition(const aiNodeAnim* channel, float time) const {
    if (channel->mNumPositionKeys == 1)
        return glm::vec3(channel->mPositionKeys[0].mValue.x, channel->mPositionKeys[0].mValue.y, channel->mPositionKeys[0].mValue.z);

    int i = FindPositionIndex(channel, time);
    int j = i + 1;
    float t1 = channel->mPositionKeys[i].mTime;
    float t2 = channel->mPositionKeys[j].mTime;
    float factor = (time - t1) / (t2 - t1);

    aiVector3D start = channel->mPositionKeys[i].mValue;
    aiVector3D end = channel->mPositionKeys[j].mValue;
    aiVector3D interp = start + factor * (end - start);
    return glm::vec3(interp.x, interp.y, interp.z);
}

glm::quat NodeAnimation::InterpolateRotation(const aiNodeAnim* channel, float time) const {
    if (channel->mNumRotationKeys == 1) {
        const aiQuaternion& q = channel->mRotationKeys[0].mValue;
        return glm::quat(q.w, q.x, q.y, q.z);
    }

    int i = FindRotationIndex(channel, time);
    int j = i + 1;
    float t1 = channel->mRotationKeys[i].mTime;
    float t2 = channel->mRotationKeys[j].mTime;
    float factor = (time - t1) / (t2 - t1);

    aiQuaternion start = channel->mRotationKeys[i].mValue;
    aiQuaternion end = channel->mRotationKeys[j].mValue;
    aiQuaternion result;
    aiQuaternion::Interpolate(result, start, end, factor);
    result.Normalize();
    return glm::quat(result.w, result.x, result.y, result.z);
}

glm::vec3 NodeAnimation::InterpolateScaling(const aiNodeAnim* channel, float time) const {
    if (channel->mNumScalingKeys == 1)
        return glm::vec3(channel->mScalingKeys[0].mValue.x, channel->mScalingKeys[0].mValue.y, channel->mScalingKeys[0].mValue.z);

    int i = FindScalingIndex(channel, time);
    int j = i + 1;
    float t1 = channel->mScalingKeys[i].mTime;
    float t2 = channel->mScalingKeys[j].mTime;
    float factor = (time - t1) / (t2 - t1);

    aiVector3D start = channel->mScalingKeys[i].mValue;
    aiVector3D end = channel->mScalingKeys[j].mValue;
    aiVector3D interp = start + factor * (end - start);
    return glm::vec3(interp.x, interp.y, interp.z);
}

// --- Index helpers ---
int NodeAnimation::FindPositionIndex(const aiNodeAnim* channel, float time) const {
    for (int i = 0; i < channel->mNumPositionKeys - 1; i++) {
        if (time < channel->mPositionKeys[i + 1].mTime)
            return i;
    }
    return channel->mNumPositionKeys - 2;
}

int NodeAnimation::FindRotationIndex(const aiNodeAnim* channel, float time) const {
    for (int i = 0; i < channel->mNumRotationKeys - 1; i++) {
        if (time < channel->mRotationKeys[i + 1].mTime)
            return i;
    }
    return channel->mNumRotationKeys - 2;
}

int NodeAnimation::FindScalingIndex(const aiNodeAnim* channel, float time) const {
    for (int i = 0; i < channel->mNumScalingKeys - 1; i++) {
        if (time < channel->mScalingKeys[i + 1].mTime)
            return i;
    }
    return channel->mNumScalingKeys - 2;
}
