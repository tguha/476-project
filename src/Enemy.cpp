#include "Enemy.h"

// --- Constructor Implementation ---
Enemy::Enemy(const glm::vec3& position, const glm::vec3& scale, const glm::vec3& rotation, float hitpoints, float moveSpeed, AssimpModel* model)
            : Entity(position, scale, rotation, hitpoints, moveSpeed, model) {}



// --- Implement any overridden virtual functions here ---
// void Enemy::move(const glm::vec3& direction) {
//     // Enemy-specific movement logic
//     Entity::move(direction); // Optionally call base class logic
// }
