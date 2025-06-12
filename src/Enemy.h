#pragma once

#include "Entity.h"
#include "BossRoomGen.h"
#include "LibraryGen.h"
#include "Pathfinder.h"
#include "Config.h"
#include "Player.h"
#include <GLFW/glfw3.h>
#include "AssimpModel.h"

#define ENEMY_HP_MAX 50.0f //change back to 200.0f

class Enemy : public Entity {
    private:
        bool hit;
        bool aggro;
        float damageTimer = 0.0f;
        glm::vec3 spawnPos;
        bool dropSpawned = false;

    protected:
        float meleeSpeed = 1.0f;
        float meleeDamage = 10.0f;
        float meleeTimer = 0.0f;
        float meleeRange = 1.0f;
        float aggroRange = 5.0f;
        float sightRange = 10.0f; // Range at which the enemy can see the player
        float territoryRadius = 10.0f; // Radius of the territory around the spawn point

    public:
        Enemy(const glm::vec3& position, float hitpoints, float moveSpeed, AssimpModel* model, const glm::vec3& scale = glm::vec3(1.0f), const glm::vec3& rotation = glm::vec3(0.0f));

        bool isHit() const;
        void setHit(bool hit);
        void attack(float damage, float deltaTime);
        void meleeAttack(Player* player, float deltaTime);
        void setAggro(bool aggro);
        bool isAggro() const;
        float getAggroRange() const;
        void setAggroRange(float range);
        float getSightRange() const;
        void setSightRange(float range);
        float getDamageTimer() const;
        void setDamageTimer(float timer);
        glm::vec3 getSpawnPos() const;
        void setSpawnPos(const glm::vec3& pos);
        AssimpModel* getModel() const;

        void setDropSpawned(const bool spawned);
        bool isDropSpawned() const;
        float deathTimer = 0.0f; // Timer for death animation
        float deathDuration = 2.0f; // Duration of the death animation

    // --- Override virtual functions if needed ---
    // virtual void move(const glm::vec3& direction) override; // Example override
    void takeDamage(float damage) override; // Example override

    virtual void moveTowardsPlayer(const glm::vec3& playerPosition, float deltaTime);
    virtual void update(Player* player, float deltaTime);

    virtual ~Enemy() = default; // Virtual destructor for proper cleanup
};