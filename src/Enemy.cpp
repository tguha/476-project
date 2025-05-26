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
}

void Enemy::moveTowardsPlayer(const glm::vec3& playerPosition, float deltaTime) {

    vec3 direction = glm::normalize(playerPosition - this->getPosition());
    direction.y = 0; // Keep the enemy on the same Y level
    direction = glm::normalize(direction); // Normalize the direction vector
    this->move(direction, deltaTime);
    this->setRotY(atan2(direction.x, direction.z) * 180.0f / glm::pi<float>()); // Rotate towards the player

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
    setDamageTimer(Config::ENEMY_HIT_DURATION);

    hitpoints -= damage;
    if (hitpoints <= 0) {
        hitpoints = 0;
        alive = false; // Set alive flag to false
        // Handle entity death visuals/logic here or in derived class override
    }
}

void Enemy::update(Player* player, float deltaTime) {
    // Drop down if not alive
    if (!isAlive()) {
        this->setPosition(this->getPosition() - glm::vec3(0.0f, 10.0f, 0.0f));
        return;
    }

    // Aggro logic
    glm::vec3 enemyPos = this->getPosition();
    glm::vec3 playerPos = player->getPosition();

    // Flatten positions to XZ plane
    glm::vec3 flatEnemyPos = glm::vec3(enemyPos.x, 0.0f, enemyPos.z);
    glm::vec3 flatPlayerPos = glm::vec3(playerPos.x, 0.0f, playerPos.z);

    // Check if player is in front of enemy forward vector
    float rotYRad = glm::radians(this->getRotY());
    glm::vec3 forward = glm::normalize(glm::vec3(sin(rotYRad), 0.0f, cos(rotYRad)));
    glm::vec3 toPlayer = glm::normalize(flatPlayerPos - flatEnemyPos);
    float isInFront = glm::dot(forward, toPlayer);

    // Enemy sight angle
    float angleThreshold = glm::cos(glm::radians(15.0f));

    // Aggro logic
    bool playerInFront = isInFront > angleThreshold;
    float distanceToPlayer = glm::distance(flatEnemyPos, flatPlayerPos);
    bool withinAggroRange = distanceToPlayer <= this->getAggroRange();

    if ((playerInFront && withinAggroRange) || this->isHit()) {
        setAggro(true);
    }

    // if (((glm::distance(this->getPosition(), player->getPosition()) <= this->getAggroRange()) && (isInFront == 1.0f)) || this->isHit()) {
    //     setAggro(true);
    // }

    if ((glm::distance(this->getPosition(), player->getPosition()) <= this->meleeRange) && this->isAggro()) {
        this->meleeAttack(player, deltaTime);
    }

    // Damage timer countdown
    if (this->getDamageTimer() > 0.0f) {
        this->setDamageTimer(this->getDamageTimer() - deltaTime);
    } else {
        this->setDamageTimer(0.0f);
    }
}

void Enemy::meleeAttack(Player* player, float deltaTime) {
    
    if (this->meleeTimer <= 0.0f) {
        player->takeDamage(this->meleeDamage);
        this->meleeTimer = this->meleeSpeed;
    } else {
        this->meleeTimer -= deltaTime;
    }
}