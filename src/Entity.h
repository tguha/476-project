#pragma once

#include "AssimpModel.h"
#include <glm/glm.hpp>
#include "Texture.h"

class Entity {
    private:
        glm::vec3 position;
        glm::vec3 scale;
        glm::vec3 rotation;
        float hitpoints;
        float moveSpeed;

    public:
        Entity(const glm::vec3& position, const glm::vec3& scale, const glm::vec3& rotation, float hitpoints, float moveSpeed);

        glm::vec3 getPosition() const;
        void setPosition(const glm::vec3 pos);
        glm::vec3 getScale() const;
        void setScale(const glm::vec3 scale);
        glm::vec3 getRotation() const;
        void setRotation(const glm::vec3 rotation);

        float getRotX() const;
        void setRotX(float rotX);
        float getRotY() const;
        void setRotY(float rotY);
        float getRotZ() const;
        void setRotZ(float rotZ);

        void takeDamage(float damage);
        void move(const glm::vec3& direction);
        bool isAlive() const;

};