#pragma once

#include "Entity.h"
#include "BossRoomGen.h"
#include "LibraryGen.h"
#include "Pathfinder.h"

#define ENEMY_HP_MAX 200.0f

class Enemy : public Entity {
    private:
        bool hit;

    public:
        Enemy(const glm::vec3& position, float hitpoints, float moveSpeed, AssimpModel* model, const glm::vec3& scale = glm::vec3(1.0f), const glm::vec3& rotation = glm::vec3(0.0f));

        bool isHit() const;
        void setHit(bool hit);
        void moveTowardsPlayer(Grid<LibraryGen::Cell>& grid, Pathfinder& pathfinder, const glm::vec3& playerPosition, float deltaTime);

    // --- Override virtual functions if needed ---
    // virtual void move(const glm::vec3& direction) override; // Example override
    // virtual void takeDamage(float damage) override; // Example override
};