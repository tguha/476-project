#include "Pathfinder.h"
#include <iostream>

// Prim's algorithm for pathfinding
// This is a simple implementation of A* pathfinding algorithm
std::vector<glm::ivec2> Pathfinder::findPath(
    const glm::ivec2& start,
    const glm::ivec2& end,
    CostFunc costFunc
) {
    // Node comparison for priority queue
    nodePool.clear(); // Clear the node pool for new pathfinding

    // Create a priority queue for open nodes
    std::priority_queue<Node*, std::vector<Node*>, Node::Compare> openSet;

    // Create the start node
    Node* startNode = nodePool.getOrCreate(start);
    startNode->cost = 0;
    startNode->heuristic = abs(end.x - start.x) + abs(end.y - start.y);
    openSet.push(startNode);

    // loop until we find the path or exhaust the open set
    while(!openSet.empty()) {
        Node* current = openSet.top();
        openSet.pop();

        if (current->closed) continue; // Skip closed nodes
        current->closed = true; // Mark the current node as closed

        // if we reached the end node, reconstruct the path
        if (current->position == end) {
            std::vector<glm::ivec2> path;
            while(current) {
                path.push_back(current->position);
                current = current->parent;
            }
            // Reverse the path to get it from start to end
            std::reverse(path.begin(), path.end());
            return path; // Return the found path
        }

        // else check all neighbors
        static const glm::ivec2 directions[] = {
            {1, 0}, {-1, 0}, {0, 1}, {0, -1}
        };

        // Loop through all 4-connected neighbors
        for (const auto& dir : directions) {
            glm::ivec2 neighborPos = current->position + dir;
            Node* neighbor = nodePool.getOrCreate(neighborPos);

            if (neighbor->closed) continue; // Skip closed nodes

            // Calculate the cost to move to the neighbor
            PathCost pathCost = costFunc(current->position, neighborPos);
            if (!pathCost.traversable) continue; // Skip non-traversable nodes

            float newCost = current->cost + pathCost.cost;

            // Check if the neighbor is already in the open set
            if (newCost < neighbor->cost) {
                neighbor->cost = newCost;
                neighbor->heuristic = abs(end.x - neighborPos.x) + abs(end.y - neighborPos.y);
                neighbor->parent = current; // Set the parent to the current node
                openSet.push(neighbor); // Add the neighbor to the open set
            }
        }
    }

    // No path found, return an empty vector
    return {};

}
