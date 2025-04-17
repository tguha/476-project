#ifndef ENEMY_H
#define ENEMY_H

#include "Entity.h"

class Enemy : public Entity {
    public:
        Enemy(AssimpModel* model, Texture *texture, const glm::vec3& position, float hitpoints, float moveSpeed);
};

#endif // ENEMY_H