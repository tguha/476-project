#ifndef PATHFINDER_H
#define PATHFINDER_H

#include <vector>
#include <queue>
#include <unordered_set>
#include <stack>
#include <functional>
#include <iostream>
#include <glm/glm.hpp>
#include <limits>
#include "Grid.h"

// a simple priority queue implementation for the pathfinding algorithm
template<typename T>
class SimplePriorityQueue {
public:
    void enqueue(T* item, float priority) {
        priorities[item] = priority;
        queue.push(PriorityItem{item, priority});
    }

    T* dequeue() {
        while (!queue.empty()) {
            PriorityItem item = queue.top();
            queue.pop();

            auto it = priorities.find(item.item);
            if (it != priorities.end() && std::abs(it->second - item.priority) < 0.001f) {
                priorities.erase(it);
                return item.item;
            }
        }
        return nullptr;
    }

    bool tryGetPriority(T* item, float& outPriority) {
        auto it = priorities.find(item);
        if (it != priorities.end()) {
            outPriority = it->second;
            return true;
        }
        return false;
    }

    void updatePriority(T* item, float newPriority) {
        priorities[item] = newPriority;
        queue.push(PriorityItem{item, newPriority});
    }

    void clear() {
        while (!queue.empty()) queue.pop();
        priorities.clear();
    }

    size_t count() const {
        return priorities.size();
    }

    bool isEmpty() const {
        return priorities.empty();
    }

private:
    struct PriorityItem {
        T* item;
        float priority;

        bool operator<(const PriorityItem& other) const {
            return priority > other.priority; // min-heap
        }
    };

    std::priority_queue<PriorityItem> queue;
    std::unordered_map<T*, float> priorities;
};


class Pathfinder {
    public:
        class Node {
            public:
                glm::ivec2 position;
                Node* previous;
                float cost;

                Node(const glm::ivec2& pos = glm::ivec2(0, 0))
                    : position(pos), previous(nullptr), cost(std::numeric_limits<float>::infinity()) {}

                // For use in hash containers
                bool operator==(const Node& other) const {
                    return position == other.position;
                }
        };

        struct NodeHash {
            size_t operator()(const Node& node) const {
                return std::hash<int>()(node.position.x) ^
                    (std::hash<int>()(node.position.y) << 1);
            }

            size_t operator()(const Node* node) const {
                return std::hash<int>()(node->position.x) ^
                    (std::hash<int>()(node->position.y) << 1);
            }
        };

        struct NodeEqual {
            bool operator()(const Node* left, const Node* right) const {
                return left->position == right->position;
            }
        };

        struct PathCost {
            bool traversable;
            float cost;
        };

        public:
            Pathfinder(const glm::ivec2& size);
            void resetNodes();
            std::vector<glm::ivec2> findPath(
                const glm::ivec2& start,
                const glm::ivec2& end,
                std::function<PathCost(Node*, Node*)> costFunc
            );
        private:
            static const std::vector<glm::ivec2> neighbors;
            Grid<Node> grid;
            SimplePriorityQueue<Node> queue;
            std::unordered_set<Node*, NodeHash, NodeEqual> closed;
            std::stack<glm::ivec2> stack;

            std::vector<glm::ivec2> reconstructPath(Node* node);
};


#endif // PATHFINDER_H