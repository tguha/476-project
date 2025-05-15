#pragma once

#include "Entity.h"
#include "BossRoomGen.h"
#include "LibraryGen.h"
#include "Pathfinder.h"
#include "Config.h"
#include "Player.h"
#include <GLFW/glfw3.h>

#define ENEMY_HP_MAX 200.0f

class Enemy : public Entity {
    private:
        bool hit;
        bool aggro;
        float damageTimer = 0.0f;
        
    protected:
        float meleeSpeed = 1.0f;
        float meleeDamage = 10.0f;
        float meleeTimer = 0.0f;
        float meleeRange = 1.0f;
        float aggroRange = 5.0f;

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
        float getDamageTimer() const;
        void setDamageTimer(float timer);
        
    // --- Override virtual functions if needed ---
    // virtual void move(const glm::vec3& direction) override; // Example override
    void takeDamage(float damage) override; // Example override

    virtual void moveTowardsPlayer(const glm::vec3& playerPosition, float deltaTime);
    virtual void update(Player* player, float deltaTime);
};