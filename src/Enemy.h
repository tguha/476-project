#pragma once

#include "Entity.h"

class Enemy : public Entity {
    public:
        Enemy(const glm::vec3& position, const glm::vec3& scale, const glm::vec3& rotation, float hitpoints, float moveSpeed);

        void attack();
};