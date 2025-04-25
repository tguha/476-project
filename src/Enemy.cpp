#include "Enemy.h"

// --- Constructor Implementation ---
Enemy::Enemy(const glm::vec3& position, float hitpoints, float moveSpeed, AssimpModel* model, const glm::vec3& scale, const glm::vec3& rotation)
            : Entity(position, hitpoints, moveSpeed, model, scale, rotation) {}



// --- Implement any overridden virtual functions here ---
// void Enemy::move(const glm::vec3& direction) {
//     // Enemy-specific movement logic
//     Entity::move(direction); // Optionally call base class logic
// }
