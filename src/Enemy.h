#pragma once

#include "Entity.h"

class Enemy : public Entity {
    public:
        Enemy(const glm::vec3& position, const glm::vec3& scale = glm::vec3(1.0f), const glm::vec3& rotation, float hitpoints, float moveSpeed, AssimpModel* model);

    // --- Override virtual functions if needed ---
    // virtual void move(const glm::vec3& direction) override; // Example override
    // virtual void takeDamage(float damage) override; // Example override
};