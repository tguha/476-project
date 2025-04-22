#include "Entity.h"

Entity::Entity(const glm::vec3& position, const glm::vec3& scale, const glm::vec3& rotation, float hitpoints, float moveSpeed)
        : position(position), scale(scale), rotation(rotation), hitpoints(hitpoints), moveSpeed(moveSpeed) {};

glm::vec3 Entity::getPosition() const {
    return position;
}

void Entity::setPosition(const glm::vec3 pos) {
    this->position = pos;
}

glm::vec3 Entity::getScale() const {
    return scale;
}

void Entity::setScale(const glm::vec3 scale) {
    this->scale = scale;
}

glm::vec3 Entity::getRotation() const {
    return rotation;
}

void Entity::setRotation(const glm::vec3 rotation) {
    this->rotation = rotation;
}

float Entity::getRotX() const {
    return rotation.x;
}
void Entity::setRotX(float rotX) {
    this->rotation.x = rotX;
}

float Entity::getRotY() const {
    return rotation.y;
}

void Entity::setRotY(float rotY) {
    this->rotation.y = rotY;
}

float Entity::getRotZ() const {
    return rotation.z;
}

void Entity::setRotZ(float rotZ) {
    this->rotation.z = rotZ;
}

void Entity::takeDamage(float damage) {
    hitpoints -= damage;
    if (hitpoints < 0) {
        hitpoints = 0;
    }

    if (!isAlive()) {
        // Handle entity death (e.g., play death animation, remove from game)
    }
}

bool Entity::isAlive() const {
    return hitpoints > 0;
}

void Entity::move(const glm::vec3& direction) {
    position += direction * moveSpeed;
    // Add any additional logic for movement (e.g., collision detection)
}