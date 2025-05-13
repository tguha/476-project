#include "IceElemental.h"

IceElemental::IceElemental(const glm::vec3& position, float hitpoints, float moveSpeed, AssimpModel* model, const glm::vec3& scale, const glm::vec3& rotation)
    : Enemy(position, hitpoints, moveSpeed, model, scale, rotation) {
        meleeTimer = Config::ICE_ELEMENTAL_MELEE_SPEED; // Timer for melee attack cooldown
        meleeRange = Config::ICE_ELEMENTAL_MELEE_RANGE; // Range for melee attack
        meleeDamage = Config::ICE_ELEMENTAL_MELEE_DAMAGE; // Damage dealt by melee attack
    }

void IceElemental::moveTowardsPlayer(const glm::vec3& playerPosition, float deltaTime) {
    vec3 direction = glm::normalize(playerPosition - this->getPosition());
    direction.y = 0; // Keep the enemy on the same Y level
    direction = glm::normalize(direction);
    this->move(direction, deltaTime);


    /* Slow rotation towards player*/

    float targetRotY = atan2(direction.x, direction.z);
    float currentRotY = this->getRotY();

    // Compute angle difference and interpolate
    float angleDiff = targetRotY - currentRotY;
    angleDiff = glm::mod(angleDiff + glm::pi<float>(), glm::two_pi<float>()) - glm::pi<float>();
    float rotationSpeed = Config::ICE_ELEMENTAL_ROTATION_SPEED;
    float maxRotation = rotationSpeed * deltaTime;

    if (fabs(angleDiff) < maxRotation) {
        currentRotY = targetRotY; // Snap if close enough
    } else {
        currentRotY += glm::sign(angleDiff) * maxRotation; // Rotate toward player
    }

    this->setRotY(currentRotY);
}

void IceElemental::update(Player* player, float deltaTime) {
    
    Enemy::update(player, deltaTime); // Call base class update

    // Bobbing animation
    float bobSpeed = 1.75f;
    float bobHeight = 0.20f;
    glm::vec3 currentPos = getPosition();
    setPosition(glm::vec3(currentPos.x, Config::ICE_ELEMENTAL_TRANS_Y + sin(glfwGetTime() * bobSpeed) * bobHeight, currentPos.z));
    // updateAABB(); // if AABB implemented

    // Move toward player if aggroed
    if (isAggro()) {
        moveTowardsPlayer(player->getPosition(), deltaTime);
    }

}