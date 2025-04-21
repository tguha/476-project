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

    resetNodes(); // Reset the nodes before finding a new path
    queue.clear(); // Clear the priority queue
    closed.clear(); // Clear the closed set

    if (!grid.inBounds(start) || !grid.inBounds(end)) {
        std::cerr << "Start or end position out of bounds!" << std::endl;
        return {};
    }

    Node& startNode = grid[start];
    startNode.cost = 0;
    queue.enqueue(&startNode, 0); // Add the start node to the queue

    // Loop until we find the path or exhaust the open set
    while (queue.count() > 0) {
        Node node = queue.dequeue(); // Get the node with the lowest cost
        Node* nodePtr = &grid[node.position];
        closed.insert(nodePtr); // Mark the node as closed

        if (node.position == end) {
            return reconstructPath(nodePtr); // Reconstruct the path if we reached the end
        }

        for (const auto& offset : neighbors) {
            glm::ivec2 neighborPos = node.position + offset;
            if (!grid.inBounds(neighborPos)) continue; // Skip out-of-bounds neighbors

            Node* neighbor = &grid[neighborPos];
            if (closed.find(neighbor) != closed.end()) continue; // Skip closed nodes

            PathCost pathCost = costFunc(nodePtr, neighbor);
            if (!pathCost.traversable) continue; // Skip non-traversable nodes

            float newCost = node.cost + pathCost.cost;
            if (newCost < neighbor->cost) {
                neighbor->previous = nodePtr; // Set the parent to the current node
                neighbor->cost = newCost; // Update the cost

                float existingPriority;
                if (queue.tryGetPriority(nodePtr, existingPriority)) {
                    queue.updatePriority(nodePtr, newCost);
                } else {
                    queue.enqueue(neighbor, neighbor->cost);
                }
            }
        }
    }

    return {}; // No path found, return an empty vector

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