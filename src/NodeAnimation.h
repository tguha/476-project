#ifndef NODE_ANIMATION_H
#define NODE_ANIMATION_H

#include <string>
#include <map>
#include <glm/glm.hpp>
#include <assimp/scene.h>
#include "AssimpModel.h"

class NodeAnimation {
public:
    NodeAnimation(const aiScene* scene, int animIndex = 0);

    glm::mat4 InterpolateNode(const std::string& nodeName, float time) const;
    const aiNodeAnim* GetNodeAnimation(const std::string& nodeName) const;

    float GetDuration() const { return duration; }
    float GetTicksPerSecond() const { return ticksPerSecond; }

    std::map<std::string, const aiNodeAnim*> nodeAnims;

private:
    float duration;
    float ticksPerSecond;

    glm::vec3 InterpolatePosition(const aiNodeAnim* channel, float time) const;
    glm::quat InterpolateRotation(const aiNodeAnim* channel, float time) const;
    glm::vec3 InterpolateScaling(const aiNodeAnim* channel, float time) const;

    int FindPositionIndex(const aiNodeAnim* channel, float time) const;
    int FindRotationIndex(const aiNodeAnim* channel, float time) const;
    int FindScalingIndex(const aiNodeAnim* channel, float time) const;
};

#endif
