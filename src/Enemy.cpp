#include "Enemy.h"

Enemy::Enemy(const glm::vec3& position, const glm::vec3& scale, const glm::vec3& rotation, float hitpoints, float moveSpeed)
            : Entity(position, scale, rotation, hitpoints, moveSpeed) {}