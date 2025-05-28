#include "Quadtree.h"
#include <stack>
#include <iostream>

Quadtree::Quadtree(glm::vec2 center, glm::vec2 half_size)
    : center(center), half_size(half_size) {
}

void Quadtree::subdivide() {
    top_right = new Quadtree(center + glm::vec2(half_size.x * 0.5f, half_size.y * 0.5f), half_size * 0.5f);
    top_left = new Quadtree(center + glm::vec2(-half_size.x * 0.5f, half_size.y * 0.5f), half_size * 0.5f);
    bottom_right = new Quadtree(center + glm::vec2(half_size.x * 0.5f, -half_size.y * 0.5f), half_size * 0.5f);
    bottom_left = new Quadtree(center + glm::vec2(-half_size.x * 0.5f, -half_size.y * 0.5f), half_size * 0.5f);
    subdivided = true;
}

bool Quadtree::insert(const QuadElement& element, int capacity) {
    // Check if the element's position is within the bounds of this quadtree node
    if (!contains(element.center)) {
        return false;
    }

    // Check if the element can b
    if (elements.size() < capacity && !subdivided) {
        // If we haven't subdivided yet and we have space, add the element
        elements.push_back(element);
        return true;
    } else {
        if (!subdivided) {
            // If we have reached capacity and haven't subdivided yet, we need to subdivide
            subdivide();
        }
    }

    if (top_right->insert(element, capacity)) return true;
    if (top_left->insert(element, capacity)) return true;
    if (bottom_right->insert(element, capacity)) return true;
    if (bottom_left->insert(element, capacity)) return true;

    return false;
}

void Quadtree::query(const glm::vec2& query_center, const glm::vec2& query_half_size, std::vector<const QuadElement*>& result) const {

    if (!intersects(query_center, query_half_size)) {
        return; // No intersection
    }

    for (const auto& element : elements) {
        if (contains(element.center)) {
            result.push_back(&element);
        }
    }

    // if (subdivided) {
    //     // Recursively query the child quadrants and combine results
    //     top_right->query(query_center, query_half_size, result);
    //     top_left->query(query_center, query_half_size, result);
    //     bottom_right->query(query_center, query_half_size, result);
    //     bottom_left->query(query_center, query_half_size, result);
    // }
    if (!subdivided) {
        return; // No need to check children if not subdivided
    }
    top_right->query(query_center, query_half_size, result);
    top_left->query(query_center, query_half_size, result);
    bottom_right->query(query_center, query_half_size, result);
    bottom_left->query(query_center, query_half_size, result);
}

void Quadtree::cleanup() {
    if (subdivided) {
        top_right->cleanup();
        top_left->cleanup();
        bottom_right->cleanup();
        bottom_left->cleanup();
    }
    elements.clear();
}
