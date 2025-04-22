#pragma once

#include "Entity.h"

class Enemy : public Entity {
public:
    // --- Constructor ---
    // MODIFIED: Now accepts model and scale, passes them to Entity constructor
    Enemy(const glm::vec3& position, float hitpoints, float moveSpeed,
        AssimpModel* model, const glm::vec3& scale = glm::vec3(1.0f));

    // --- Override virtual functions if needed ---
    // virtual void move(const glm::vec3& direction) override; // Example override
    // virtual void takeDamage(float damage) override; // Example override
};