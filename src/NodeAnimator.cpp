#include "NodeAnimator.h"

NodeAnimator::NodeAnimator(std::shared_ptr<NodeAnimation> animation)
    : animation(animation), currentTime(0.0f) {}

void NodeAnimator::Update(float dt) {
    if (!animation) return;

    float ticksPerSecond = animation->GetTicksPerSecond();
    float duration = animation->GetDuration();

    currentTime += dt * ticksPerSecond;
    currentTime = fmod(currentTime, duration);

    // Rebuild nodeTransforms each frame
    nodeTransforms.clear();
    for (const auto& [nodeName, _] : animation->nodeAnims) {
        nodeTransforms[nodeName] = animation->InterpolateNode(nodeName, currentTime);
    }
}

glm::mat4 NodeAnimator::GetNodeTransform(const std::string& nodeName) const {
    auto it = nodeTransforms.find(nodeName);
    return (it != nodeTransforms.end()) ? it->second : glm::mat4(1.0f);
}
