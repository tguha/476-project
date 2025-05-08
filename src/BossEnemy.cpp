#include "BossEnemy.h"

BossEnemy::BossEnemy(const glm::vec3& position, float hitpoints, AssimpModel* model, const glm::vec3& scale,
const glm::vec3& rotation, float specialAttackCooldown)
    : Enemy(position, BOSS_HP_MAX, 0.0f ,model, scale, rotation), specialAttackCooldown(specialAttackCooldown) {
        this->setRotY(0.0f); // Initialize rotation to face forward
        this->enraged = false; // Initialize enraged state
    }

void BossEnemy::changePhase() {
    if (this->getHitpoints() >= BOSS_ENRAGED_HP && !this->enraged) {
        this->enraged = true;
        this->phase = BossPhase::PHASE_2; // Change to phase 2
    } else if (this->getHitpoints() <= BOSS_HP_MAX / 2 && this->phase == BossPhase::PHASE_2) {
        this->phase = BossPhase::PHASE_3; // Change to phase 3
    }
}

void BossEnemy::lookAtPlayer(const glm::vec3& playerPosition) {
    glm::vec3 direction = glm::normalize(playerPosition - this->getPosition());
    float angle = glm::degrees(atan2(direction.x, direction.z)); // Calculate angle in degrees
    if (angle < 0) {
        angle += 360.0f; // Normalize angle to [0, 360)
    }
    this->setRotY(glm::radians(angle)); // Set rotation in radians


    // this->setRotY(atan2(direction.x, direction.z)); // Rotate towards player
}