#ifndef ENTITY_H
#define ENTITY_H

#include "AssimpModel.h"
#include <glm/glm.hpp>
#include "Texture.h"

class Entity {
    private:
        AssimpModel *model;
        Texture *texture;
        glm::vec3 position;
        float hitpoints;
        float moveSpeed;

    public:
        Entity(AssimpModel* model, Texture *texture, const glm::vec3& position, float hitpoints, float moveSpeed);

        glm::vec3 getPosition() const;
        void setPosition(const glm::vec3& pos);
        void takeDamage(float damage);
        bool isAlive() const;

};

#endif // ENTITY_H