#ifndef PATHFINDER_H
#define PATHFINDER_H

#include <vector>
#include <functional>
#include <queue>
#include <unordered_map>
#include <memory>
#include <glm/glm.hpp>

namespace std {
    template<>
    struct hash<glm::ivec2> {
        size_t operator()(const glm::ivec2& vec) const {
            size_t h1 = std::hash<int>()(vec.x);
            size_t h2 = std::hash<int>()(vec.y);

            return h1 ^ (h2 << 1); // XOR and shift to combine hashes
        }
    };
};

struct PathCost {
    bool traversable;
    float cost;
};

class Pathfinder {
    public:
        using CostFunc = std::function<PathCost(const glm::ivec2&, const glm::ivec2&)>;

        std::vector<glm::ivec2> findPath(
            const glm::ivec2& start,
            const glm::ivec2& end,
            CostFunc constFunc
        );
    private:
        struct Node {
            glm::ivec2 position;
            float cost;
            float heuristic;
            Node* parent;
            bool closed;

            // Required for priority queue
            struct Compare {
                bool operator()(Node* a, Node* b) {
                    return (a->cost + a->heuristic) > (b->cost + b->heuristic);
                }
            };
        };

        struct NodePool {
            std::unordered_map<glm::ivec2, std::unique_ptr<Node>> nodes;

            Node* getOrCreate(const glm::ivec2& pos) {
                auto& node = nodes[pos];
                if (!node) {
                    node = std::make_unique<Node>();
                    node->position = pos;
                    node->closed = false;
                }
                return node.get();
            }

            void clear() {
                nodes.clear();
            }
        };

        NodePool nodePool;
};

#endif // PATHFINDER_H