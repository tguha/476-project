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
    std::cout << "[Pathfinder] Starting pathfinding from (" << start.x << ", " << start.y 
              << ") to (" << end.x << ", " << end.y << ")" << std::endl;

    resetNodes(); 
    queue.clear(); 
    closed.clear(); 

    if (!grid.inBounds(start) || !grid.inBounds(end)) {
        std::cerr << "[Pathfinder] Error: Start or end position out of bounds!" << std::endl;
        return {};
    }

    Node& startNode = grid[start];
    startNode.cost = 0;
    queue.enqueue(&startNode, 0); 
    std::cout << "[Pathfinder] Enqueued start node at (" << start.x << ", " << start.y << ")" << std::endl;

    while (queue.count() > 0) {
        Node* nodePtr = queue.dequeue();
        if (!nodePtr) {
            std::cerr << "[Pathfinder] Queue returned nullptr! Aborting." << std::endl;
            break;
        }

        closed.insert(nodePtr); 

        std::cout << "[Pathfinder] Dequeued node (" << nodePtr->position.x << ", " << nodePtr->position.y 
                  << "), cost = " << nodePtr->cost << std::endl;

        if (nodePtr->position == end) {
            std::cout << "[Pathfinder] Goal reached at (" << nodePtr->position.x << ", " << nodePtr->position.y << ")" << std::endl;
            return reconstructPath(nodePtr);
        }

        for (const auto& offset : neighbors) {
            glm::ivec2 neighborPos = nodePtr->position + offset;
            if (!grid.inBounds(neighborPos)) {
                std::cout << "[Pathfinder] Neighbor (" << neighborPos.x << ", " << neighborPos.y << ") out of bounds, skipping." << std::endl;
                continue;
            }

            Node* neighbor = &grid[neighborPos];
            if (closed.find(neighbor) != closed.end()) {
                std::cout << "[Pathfinder] Neighbor (" << neighborPos.x << ", " << neighborPos.y << ") already closed, skipping." << std::endl;
                continue;
            }

            PathCost pathCost = costFunc(nodePtr, neighbor);
            if (!pathCost.traversable) {
                std::cout << "[Pathfinder] Neighbor (" << neighborPos.x << ", " << neighborPos.y << ") not traversable, skipping." << std::endl;
                continue;
            }

            float newCost = nodePtr->cost + pathCost.cost;
            if (newCost < neighbor->cost) {
                std::cout << "[Pathfinder] Updating neighbor (" << neighborPos.x << ", " << neighborPos.y 
                          << ") with new cost " << newCost << std::endl;

                neighbor->previous = nodePtr;
                neighbor->cost = newCost;

                float existingPriority;
                if (queue.tryGetPriority(neighbor, existingPriority)) {
                    std::cout << "[Pathfinder] Updating priority of neighbor in queue." << std::endl;
                    queue.updatePriority(neighbor, newCost);
                } else {
                    std::cout << "[Pathfinder] Enqueuing new neighbor (" << neighborPos.x << ", " << neighborPos.y 
                              << ") with cost " << newCost << std::endl;
                    queue.enqueue(neighbor, neighbor->cost);
                }
            }
        }
    }

    std::cout << "[Pathfinder] No path found." << std::endl;
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