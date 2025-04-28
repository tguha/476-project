#include "Entity.h"

// --- Constructor Implementation ---
Entity::Entity(const glm::vec3& startPosition, float hp, float speed, AssimpModel* model, const glm::vec3& scale, const glm::vec3& rotation)
    : position(startPosition),
    hitpoints(hp),
    moveSpeed(speed),
    alive(true), // Start alive
    collisionModel(model),
    collisionScale(scale),
    rotation(rotation),
    aabbMin(startPosition), // Initialize AABB roughly
    aabbMax(startPosition)
{
    if (hitpoints <= 0) { // Ensure entity starts dead if given 0 or less HP
        alive = false;
    }
    updateAABB(); // Calculate initial AABB based on provided model and scale
}

// --- Getter Implementations ---
glm::vec3 Entity::getPosition() const {
    return position;
}

float Entity::getHitpoints() const {
    return hitpoints;
}

bool Entity::isAlive() const {
    // Check both the flag and hitpoints for robustness
    return alive && hitpoints > 0;
}

glm::vec3 Entity::getAABBMin() const {
    return aabbMin;
}

glm::vec3 Entity::getAABBMax() const {
    return aabbMax;
}

// --- Setter Implementation ---
void Entity::setPosition(const glm::vec3 pos) {
    this->position = pos;
    updateAABB();
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

// --- Action Implementations ---
void Entity::takeDamage(float damage) {
    if (!alive) return; // Can't damage dead entities

    hitpoints -= damage;
    if (hitpoints <= 0) {
        hitpoints = 0;
        alive = false; // Set alive flag to false
        // Handle entity death visuals/logic here or in derived class override
    }
}

void Entity::move(const glm::vec3& direction) {
    // Basic movement - doesn't include collision checks here
    // Collision should ideally be handled before calling setPosition
    glm::vec3 nextPos = position + direction * moveSpeed; // Calculate potential next position
    // TODO: Add collision detection logic here or in the game loop *before* calling setPosition
    setPosition(nextPos); // Update position and AABB
}

// --- Collision Update Implementation ---
void Entity::updateAABB() {
    if (!collisionModel) {
        // No model, AABB is just a point at the entity's position
        aabbMin = position;
        aabbMax = position;
        return;
    }

    // 1. Get Local AABB from Model
    glm::vec3 localMin = collisionModel->getBoundingBoxMin();
    glm::vec3 localMax = collisionModel->getBoundingBoxMax();

    // 2. Scale the Local AABB
    // Apply the specific collisionScale stored for this entity
    glm::vec3 scaledMin = localMin * collisionScale;
    glm::vec3 scaledMax = localMax * collisionScale;

    // Ensure min < max after scaling (handles potential negative scales)
    for (int i = 0; i < 3; ++i) {
        if (scaledMin[i] > scaledMax[i]) {
            std::swap(scaledMin[i], scaledMax[i]);
        }
    }

    // 3. Get World Transform (assuming only translation for now)
    // If enemies rotate, you'll need to include rotation here
    glm::mat4 transform = glm::translate(glm::mat4(1.0f), position);

    // 4. Calculate World AABB using the static helper
    calculateWorldAABB(scaledMin, scaledMax, transform, aabbMin, aabbMax);

    // Optional Debug Print:
    // std::cout << "Updated Enemy AABB Min: (" << aabbMin.x << "," << aabbMin.y << "," << aabbMin.z << ")" << std::endl;
    // std::cout << "Updated Enemy AABB Max: (" << aabbMax.x << "," << aabbMax.y << "," << aabbMax.z << ")" << std::endl;
}