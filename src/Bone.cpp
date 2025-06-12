#include "Bone.h"
#include <iostream>

#define GLM_ENABLE_EXPERIMENTAL

Bone::Bone(const std::string& name, int ID, const aiNodeAnim* channel)
    :
    m_Name(name),
    m_ID(ID),
    m_LocalTransform(1.0f)
{
    m_NumPositions = channel->mNumPositionKeys;

    for (int i = 0; i < m_NumPositions; ++i)
    {
        aiVector3D aiPosition = channel->mPositionKeys[i].mValue;
        float timeStamp = channel->mPositionKeys[i].mTime;
        KeyPosition data;
        data.position = AssimpGLMHelpers::GetGLMVec(aiPosition);
        data.timeStamp = timeStamp;
        m_Positions.push_back(data);
    }

    m_NumRotations = channel->mNumRotationKeys;

    for (int i = 0; i < m_NumRotations; ++i)
    {
        aiQuaternion aiOrientation = channel->mRotationKeys[i].mValue;
        float timeStamp = channel->mRotationKeys[i].mTime;
        KeyRotation data;
        data.orientation = AssimpGLMHelpers::GetGLMQuat(aiOrientation);
        data.timeStamp = timeStamp;
        m_Rotations.push_back(data);
    }

    m_NumScalings = channel->mNumScalingKeys;

    for (int i = 0; i < m_NumScalings; ++i)
    {
        aiVector3D aiScale = channel->mScalingKeys[i].mValue;
        float timeStamp = channel->mScalingKeys[i].mTime;
        KeyScale data;
        data.scale = AssimpGLMHelpers::GetGLMVec(aiScale);
        data.timeStamp = timeStamp;
        m_Scales.push_back(data);
    }
}

void Bone::Update(float animationTime)
{

    if (m_NumPositions == 0 || m_NumRotations == 0 || m_NumScalings == 0)
    {
        m_LocalTransform = glm::mat4(1.0f);
        return;
    }

    glm::mat4 translation = InterpolatePosition(animationTime);
    glm::mat4 rotation = InterpolateRotation(animationTime);
    glm::mat4 scale = InterpolateScaling(animationTime);

    m_LocalTransform = translation * rotation * scale;
}

int Bone::GetPositionIndex(float animationTime)
{
    for (int i = 0; i < m_NumPositions - 1; ++i)
    {
        if (animationTime < m_Positions[i + 1].timeStamp)
        {
            return i;
        }
    }

    // assert(0);
    return m_NumPositions - 1; // Return the last index if no match found
}

int Bone::GetRotationIndex(float animationTime)
{
    for (int i = 0; i < m_NumRotations - 1; ++i)
    {
        if (animationTime < m_Rotations[i + 1].timeStamp)
        {
            return i;
        }
    }

    // assert(0);
    return m_NumRotations - 1; // Return the last index if no match found
}

int Bone::GetScaleIndex(float animationTime)
{
    for (int i = 0; i < m_NumScalings - 1; ++i)
    {
        if (animationTime < m_Scales[i + 1].timeStamp)
        {
            return i;
        }
    }

    // assert(0);
    return m_NumScalings - 1; // Return the last index if no match found
}

float Bone::GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime)
{
    float scaleFactor = 0.0f;
    float midWayLength = animationTime - lastTimeStamp;
    float framesDiff = nextTimeStamp - lastTimeStamp;
    scaleFactor = midWayLength / framesDiff;
    return scaleFactor;
}

//glm::mat4 Bone::InterpolatePosition(float animationTime)
//{
//    if (1 == m_NumPositions)
//    {
//        return glm::translate(glm::mat4(1.0f), m_Positions[0].position);
//    }
//
//    int p0Index = GetPositionIndex(animationTime);
//    int p1Index = p0Index + 1;
//    float scaleFactor = GetScaleFactor(m_Positions[p0Index].timeStamp, m_Positions[p1Index].timeStamp, animationTime);
//    glm::vec3 finalPosition = glm::mix(m_Positions[p0Index].position, m_Positions[p1Index].position, scaleFactor);
//    return glm::translate(glm::mat4(1.0f), finalPosition);
//}

glm::mat4 Bone::InterpolatePosition(float animationTime)
{
    const auto& keys = m_Positions; // our vector<KeyPosition>
    size_t N = keys.size();
    if (N == 0) {
        // no keys => identity
        return glm::mat4(1.0f);
    }
    if (N == 1) {
        // single key => just return its translation
        return glm::translate(glm::mat4(1.0f), keys[0].position);
    }

    // clamp times outside the key range
    if (animationTime <= keys.front().timeStamp) {
        return glm::translate(glm::mat4(1.0f), keys.front().position);
    }
    if (animationTime >= keys.back().timeStamp) {
        return glm::translate(glm::mat4(1.0f), keys.back().position);
    }

    // find the interval [i, i+1] such that
    // keys[i].timeStamp <= animationTime < keys[i+1].timeStamp
    size_t i = 0;
    for (; i + 1 < N; ++i) {
        if (animationTime < keys[i + 1].timeStamp)
            break;
    }
    // now i+1 < N by construction

    // compute interpolation factor in [0,1]
    float t0 = keys[i].timeStamp;
    float t1 = keys[i + 1].timeStamp;
    float frac = (animationTime - t0) / (t1 - t0);
    frac = glm::clamp(frac, 0.0f, 1.0f);

    // lerp the positions
    glm::vec3 P = glm::mix(keys[i].position,
        keys[i + 1].position,
        frac);

    return glm::translate(glm::mat4(1.0f), P);
}


//glm::mat4 Bone::InterpolateRotation(float animationTime) {
//    if (1 == m_NumRotations) {
//        auto rotation = glm::normalize(m_Rotations[0].orientation);
//        return glm::toMat4(rotation);
//    }
//
//    int p0Index = GetRotationIndex(animationTime);
//    int p1Index = p0Index + 1;
//    float scaleFactor = GetScaleFactor(m_Rotations[p0Index].timeStamp, m_Rotations[p1Index].timeStamp, animationTime);
//    glm::quat finalRotation = glm::slerp(m_Rotations[p0Index].orientation, m_Rotations[p1Index].orientation, scaleFactor);
//    finalRotation = glm::normalize(finalRotation);
//
//    return glm::toMat4(finalRotation);
//}

glm::mat4 Bone::InterpolateRotation(float animationTime)
{
    const auto& keys = m_Rotations;            // your vector<KeyRotation>
    size_t N = keys.size();

    // --- handle trivial cases ---
    if (N == 0) {
        // no keys => identity rotation
        return glm::mat4(1.0f);
    }
    if (N == 1) {
        // single key => just return its orientation
        glm::quat q = glm::normalize(keys[0].orientation);
        return glm::toMat4(q);
    }

    // --- clamp animationTime to key range ---
    if (animationTime <= keys.front().timeStamp) {
        glm::quat q = glm::normalize(keys.front().orientation);
        return glm::toMat4(q);
    }
    if (animationTime >= keys.back().timeStamp) {
        glm::quat q = glm::normalize(keys.back().orientation);
        return glm::toMat4(q);
    }

    // --- find interval [i, i+1] containing animationTime ---
    size_t i = 0;
    for (; i + 1 < N; ++i) {
        if (animationTime < keys[i + 1].timeStamp)
            break;
    }
    // now i+1 < N guaranteed

    size_t j = i + 1;
    float  t0 = keys[i].timeStamp;
    float  t1 = keys[j].timeStamp;
    float  alpha = (animationTime - t0) / (t1 - t0);
    alpha = glm::clamp(alpha, 0.0f, 1.0f);

    // --- spherical interpolate and normalize ---
    glm::quat R = glm::slerp(keys[i].orientation,
        keys[j].orientation,
        alpha);
    R = glm::normalize(R);

    return glm::toMat4(R);
}


//glm::mat4 Bone::InterpolateScaling(float animationTime) {
//    if (1 == m_NumScalings) {
//        return glm::scale(glm::mat4(1.0f), m_Scales[0].scale);
//    }
//
//    int p0Index = GetScaleIndex(animationTime);
//    int p1Index = p0Index + 1;
//    float scaleFactor = GetScaleFactor(m_Scales[p0Index].timeStamp, m_Scales[p1Index].timeStamp, animationTime);
//    glm::vec3 finalScale = glm::mix(m_Scales[p0Index].scale, m_Scales[p1Index].scale, scaleFactor);
//    return glm::scale(glm::mat4(1.0f), finalScale);
//}

glm::mat4 Bone::InterpolateScaling(float animationTime)
{
    const auto& keys = m_Scales;               // your vector<KeyScale>
    size_t N = keys.size();

    // --- handle trivial cases ---
    if (N == 0) {
        // no keys => identity (no scale)
        return glm::mat4(1.0f);
    }
    if (N == 1) {
        // single key => just return its scale
        return glm::scale(glm::mat4(1.0f), keys[0].scale);
    }

    // --- clamp animationTime to key range ---
    if (animationTime <= keys.front().timeStamp) {
        return glm::scale(glm::mat4(1.0f), keys.front().scale);
    }
    if (animationTime >= keys.back().timeStamp) {
        return glm::scale(glm::mat4(1.0f), keys.back().scale);
    }

    // --- find interval [i, i+1] containing animationTime ---
    size_t i = 0;
    for (; i + 1 < N; ++i) {
        if (animationTime < keys[i + 1].timeStamp)
            break;
    }
    // now i+1 < N guaranteed

    size_t j = i + 1;
    float  t0 = keys[i].timeStamp;
    float  t1 = keys[j].timeStamp;
    float  alpha = (animationTime - t0) / (t1 - t0);
    alpha = glm::clamp(alpha, 0.0f, 1.0f);

    // --- interpolate the scale vector ---
    glm::vec3 S = glm::mix(keys[i].scale,
        keys[j].scale,
        alpha);

    return glm::scale(glm::mat4(1.0f), S);
}


// midwayLength = animationTime - lastTimeStamp
// framesDiff = nextTimeStamp - lastTimeStamp
// scaleFactor = midwayLength / framesDiff

