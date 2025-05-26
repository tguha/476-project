#ifndef QUADTREE_H
#define QUADTREE_H

#include <vector>
#include <glm/glm.hpp>
#include <functional>

struct QuadElement {
    // Unique identifier for the element
    int id;
    // Center of the element
    glm::vec2 center;


    // store the AABB of the element
    glm::vec3 aabb_min;
    glm::vec3 aabb_max;

    QuadElement() = default;
    QuadElement(int id, const glm::vec2& center, const glm::vec3& aabb_min, const glm::vec3& aabb_max)
        : id(id), aabb_min(aabb_min), aabb_max(aabb_max), center(center) {}
};

class Quadtree {
    public:
        Quadtree(glm::vec2 center, glm::vec2 half_size, int max_depth = 8);

        bool contains(const glm::vec2& point) const {
            return (point.x >= center.x - half_size.x && point.x <= center.x + half_size.x &&
                    point.y >= center.y - half_size.y && point.y <= center.y + half_size.y);
        }

        bool insert(const QuadElement& element, int capacity = 8);
        void cleanup();

        void query(const glm::vec2& center, const glm::vec2& half_size, std::vector<const QuadElement*>& results) const;

        int getElementCount() const {
            int size = elements.size();
            if (subdivided) {
                size += top_right->getElementCount();
                size += top_left->getElementCount();
                size += bottom_right->getElementCount();
                size += bottom_left->getElementCount();
            }
            return size;
        }
    private:
        void subdivide();
        bool intersects(const glm::vec2& center, const glm::vec2& half_size) const {
            return !(
                center.x - half_size.x > this->center.x + this->half_size.x ||
                center.x + half_size.x < this->center.x - this->half_size.x ||
                center.y - half_size.y > this->center.y + this->half_size.y ||
                center.y + half_size.y < this->center.y - this->half_size.y
            );
        }

        // glm::vec2 root_min, root_max;
        // glm::vec2 root_center, root_half_size;

        glm::vec2 center, half_size;

        int max_depth;
        // int free_node = -1;

        bool subdivided = false;

        std::vector<QuadElement> elements;
        Quadtree *top_right;
        Quadtree *top_left;
        Quadtree *bottom_right;
        Quadtree *bottom_left;
};

#endif