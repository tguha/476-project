#ifndef NODE_ANIMATOR_H
#define NODE_ANIMATOR_H

#include <memory>
#include <string>
#include <map>
#include <glm/glm.hpp>
#include "NodeAnimation.h"

class NodeAnimator {
public:
    NodeAnimator(std::shared_ptr<NodeAnimation> animation);

    void Update(float dt);
    glm::mat4 GetNodeTransform(const std::string& nodeName) const;

private:
    float currentTime;
    std::shared_ptr<NodeAnimation> animation;
    std::map<std::string, glm::mat4> nodeTransforms;
};

#endif
