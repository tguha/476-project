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
        bool alive;

        // --- Collision Data ---
        glm::vec3 aabbMin;
        glm::vec3 aabbMax;
        glm::vec3 collisionScale;    // Scale applied to the collision model's AABB
        AssimpModel* collisionModel; // Pointer to the model used for collision bounds
    public:
        Entity(const glm::vec3& position, float hitpoints, float moveSpeed, AssimpModel* model, const glm::vec3& scale, const glm::vec3& rotation);

        virtual ~Entity() = default;

        // --- Getters ---
        glm::vec3 getPosition() const;
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

        float getHitpoints() const;
        bool isAlive() const;
        glm::vec3 getAABBMin() const; // Getter for AABB
        glm::vec3 getAABBMax() const; // Getter for AABB

        // --- Setters ---
        virtual void setPosition(const glm::vec3 pos); // Update AABB when position changes

        // --- Actions ---
        virtual void takeDamage(float damage);
        virtual void move(const glm::vec3& direction); // Keep move virtual if needed

        // --- Collision Update ---
        virtual void updateAABB(); // Make it virtual for potential overrides

protected:
    // Helper to calculate world AABB from local bounds and transform
    // (Could be static or moved to a utility header)
    static void calculateWorldAABB(const glm::vec3& localMin, const glm::vec3& localMax, const glm::mat4& transform, glm::vec3& outWorldMin, glm::vec3& outWorldMax) {
        outWorldMin = glm::vec3(std::numeric_limits<float>::max());
        outWorldMax = glm::vec3(-std::numeric_limits<float>::max());
        glm::vec3 corners[8] = {
            {localMin.x, localMin.y, localMin.z}, {localMax.x, localMin.y, localMin.z},
            {localMin.x, localMax.y, localMin.z}, {localMax.x, localMax.y, localMin.z},
            {localMin.x, localMin.y, localMax.z}, {localMax.x, localMin.y, localMax.z},
            {localMin.x, localMax.y, localMax.z}, {localMax.x, localMax.y, localMax.z}
        };
        for (int i = 0; i < 8; ++i) {
            glm::vec4 transformed = transform * glm::vec4(corners[i], 1.0f);
            outWorldMin = glm::min(outWorldMin, glm::vec3(transformed));
            outWorldMax = glm::max(outWorldMax, glm::vec3(transformed));
        }
    }
};