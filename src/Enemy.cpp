#include "Enemy.h"
#include "Grid.h"

// --- Constructor Implementation ---
Enemy::Enemy(const glm::vec3& position, float hitpoints, float moveSpeed, AssimpModel* model, const glm::vec3& scale, const glm::vec3& rotation)
            : Entity(position, hitpoints, moveSpeed, model, scale, rotation) {}


bool Enemy::isHit() const {
    return this->hit;
}

void Enemy::setHit(bool hit) {
    this->hit = hit;
    setDamageTimer(Config::ENEMY_HIT_DURATION);
}

void Enemy::moveTowardsPlayer(Grid<LibraryGen::Cell>& grid, Pathfinder& pathfinder, const glm::vec3& playerPosition, float deltaTime) {
    
    vec3 direction = glm::normalize(playerPosition - this->getPosition());
    direction.y = 0; // Keep the enemy on the same Y level
    direction = glm::normalize(direction); // Normalize the direction vector
    this->move(direction, deltaTime);
    this->setRotY(atan2(direction.x, direction.z) * 180.0f / M_PI); // Rotate towards the player
}

void Enemy::attack(float damage, float deltaTime) {
    // Implement attack logic here
    std::cout << "Enemy attacks with damage: " << damage << std::endl;

}

void Enemy::setAggro(bool aggro) {
    this->aggro = aggro;
}

bool Enemy::isAggro() const {
    return this->aggro;
}

float Enemy::getAggroRange() const {
    return this->aggroRange;
}

void Enemy::setAggroRange(float range) {
    this->aggroRange = range;
}

float Enemy::getDamageTimer() const {
    return this->damageTimer;
}

void Enemy::setDamageTimer(float timer) {
    this->damageTimer = timer;
}

void Enemy::takeDamage(float damage) {
    if (!isAlive()) return; // Can't damage dead entities

    this->setHit(true);

    hitpoints -= damage;
    if (hitpoints <= 0) {
        hitpoints = 0;
        alive = false; // Set alive flag to false
        // Handle entity death visuals/logic here or in derived class override
    }
}