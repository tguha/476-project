#include "Enemy.h"

Enemy::Enemy(AssimpModel* model, Texture *texture, const glm::vec3& position, float hitpoints, float moveSpeed)
            : Entity(model, texture, position, hitpoints, moveSpeed) {}