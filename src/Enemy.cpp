#include "Enemy.h"

// --- Constructor Implementation ---
// MODIFIED: Passes arguments up to the Entity base class constructor
Enemy::Enemy(const glm::vec3& position, float hitpoints, float moveSpeed,
    AssimpModel* model, const glm::vec3& scale)
    : Entity(position, hitpoints, moveSpeed, model, scale) // Call base constructor
{
    // Add any Enemy-specific initialization here
}

// --- Implement any overridden virtual functions here ---
// void Enemy::move(const glm::vec3& direction) {
//     // Enemy-specific movement logic
//     Entity::move(direction); // Optionally call base class logic
// }