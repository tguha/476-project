#include "Pathfinder.h"

const std::vector<glm::ivec2> Pathfinder::neighbors = {
    glm::ivec2(1, 0),
    glm::ivec2(-1, 0),
    glm::ivec2(0, 1),
    glm::ivec2(0, -1),
};

Pathfinder::Pathfinder(const glm::ivec2& size) : grid(size) {
    // Initialize the grid with default values
    for (int y = 0; y < size.y; ++y) {
        for (int x = 0; x < size.x; ++x) {
            grid.at(x, y) = Node(glm::ivec2(x, y));
        }
    }
}



void Pathfinder::resetNodes() {
    auto size = grid.getSize();
    for (int x = 0; x < size.x; x++) {
        for (int y = 0; y < size.y; y++) {
            Node& node = grid.at(x, y);
            node.previous = nullptr;
            node.cost = std::numeric_limits<float>::infinity();
        }
    }
}

std::vector<glm::ivec2> Pathfinder::findPath(const glm::ivec2& start, const glm::ivec2& end, std::function<PathCost(Node*, Node*)> costFunc) {
    // std::cout << "[Pathfinder] Starting pathfinding from (" << start.x << ", " << start.y
    //           << ") to (" << end.x << ", " << end.y << ")" << std::endl;

    resetNodes();
    queue.clear();
    closed.clear();

    if (!grid.inBounds(start) || !grid.inBounds(end)) {
        return {};
    }

    Node& startNode = grid[start];
    startNode.cost = 0;
    queue.enqueue(&startNode, 0);

    while (queue.count() > 0) {
        Node* nodePtr = queue.dequeue();
        if (!nodePtr) {
            break;
        }

        closed.insert(nodePtr);

        if (nodePtr->position == end) {
            return reconstructPath(nodePtr);
        }

        for (const auto& offset : neighbors) {
            glm::ivec2 neighborPos = nodePtr->position + offset;
            if (!grid.inBounds(neighborPos)) {
                continue;
            }

            Node* neighbor = &grid[neighborPos];
            if (closed.find(neighbor) != closed.end()) {
                continue;
            }

            PathCost pathCost = costFunc(nodePtr, neighbor);
            if (!pathCost.traversable) {
                continue;
            }

            float newCost = nodePtr->cost + pathCost.cost;
            if (newCost < neighbor->cost) {
                neighbor->previous = nodePtr;
                neighbor->cost = newCost;

                float existingPriority;
                if (queue.tryGetPriority(neighbor, existingPriority)) {
                    queue.updatePriority(neighbor, newCost);
                } else {
                    queue.enqueue(neighbor, neighbor->cost);
                }
            }
        }
    }
    return {};
}



std::vector<glm::ivec2> Pathfinder::reconstructPath(Node* node) {
    std::vector<glm::ivec2> result;

    while (!stack.empty()) {
        stack.pop(); // Clear the stack before reconstructing the path
    }

    while (node != nullptr) {
        stack.push(node->position); // Push the node position onto the stack
        node = node->previous; // Move to the previous node
    }

    result.reserve(stack.size()); // Reserve space for the result vector
    while (!stack.empty()) {
        result.push_back(stack.top()); // Pop the node position from the stack and add it to the result
        stack.pop(); // Remove the node from the stack
    }

    return result; // Return the reconstructed path
}