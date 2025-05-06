#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "Spline.h"
#include "AssimpModel.h"
#include "Config.h"

// --- Enums ---
enum class BookState {
    ON_SHELF,
    FALLING,
    LANDED,
    OPENING,
    OPENED
};

enum class OrbState {
    SPAWNING,
    LEVITATING,
    IDLE,
    COLLECTED
};

enum class Man_State {
    WALKING,
    STANDING
};


// --- Structs ---
struct SpellProjectile {
    glm::vec3 position;
    glm::vec3 direction;
    glm::vec3 scale = glm::vec3(0.05f, 0.05f, 0.6f);
    float speed = 20.0f;
    float lifetime = 2.0f;
    float spawnTime = 0.0f;
    bool active = true;
    AssimpModel* model = nullptr;

    glm::vec3 aabbMin;
    glm::vec3 aabbMax;
    glm::mat4 transform;

    SpellProjectile(glm::vec3 startPos, glm::vec3 dir, float time, AssimpModel* mdl)
        : position(startPos), direction(normalize(dir)), spawnTime(time), model(mdl), transform(1.0f) {
    }
};

// --- Classes ---
class Book {
public:
    vec3 initialPosition;
    vec3 position;
    vec3 scale;
    quat orientation;
    BookState state = BookState::ON_SHELF;
    Spline* fallSpline = nullptr;
    float fallStartTime = 0.0f;
    float openAngle = 0.0f;
    float maxOpenAngle = glm::radians(80.0f);
    float openSpeed = glm::radians(120.0f);
    AssimpModel* bookModel;
    AssimpModel* orbModel;
    vec3 orbColor;
    float orbScale = 0.1f;
    bool orbSpawned = false;

    Book(AssimpModel* bookMdl, AssimpModel* orbMdl, const glm::vec3& pos, const glm::vec3& scl, const glm::quat& orient, const glm::vec3& orbClr)
        : initialPosition(pos), position(pos), scale(scl), orientation(orient),
        bookModel(bookMdl), orbModel(orbMdl), orbColor(orbClr) {
    }

    ~Book() {
        delete fallSpline;
    }

    void startFalling(float groundY, const glm::vec3& playerPos) {
        if (state != BookState::ON_SHELF) return;
        state = BookState::FALLING;
        fallStartTime = (float)glfwGetTime();

        glm::vec3 endPosition;
        endPosition.y = groundY + (scale.y * 0.5f);
        glm::vec3 dirToBook = playerPos - initialPosition;
        dirToBook.y = 0.0f;
        float distSq = dot(dirToBook, dirToBook);
        if (distSq < 0.01f) {
            dirToBook = glm::vec3(0.0f, 0.0f, 1.0f);
        }
        else {
            dirToBook = normalize(dirToBook);
        }
        float landingDistance = 3.0f;
        float randomSpread = 0.75f;
        endPosition.x = initialPosition.x + dirToBook.x * landingDistance + Config::randFloat(-randomSpread, randomSpread);
        endPosition.z = initialPosition.z + dirToBook.z * landingDistance + Config::randFloat(-randomSpread, randomSpread);

        glm::vec3 controlPoint = (initialPosition + endPosition) * 0.5f;
        controlPoint.y = initialPosition.y + 3.0f;
        controlPoint.x += Config::randFloat(-1.5f, 1.5f);

        float fallDuration = 0.4f;
        delete fallSpline;
        fallSpline = new Spline(initialPosition, controlPoint, endPosition, fallDuration);
    }

    void update(float deltaTime, float groundY) {
        switch (state) {
        case BookState::FALLING:
            if (fallSpline) {
                fallSpline->update(deltaTime);
                position = fallSpline->getPosition();
                if (fallSpline->isDone()) {
                    state = BookState::LANDED;
                    position.y = groundY + (scale.y * 0.5f);
                    delete fallSpline;
                    fallSpline = nullptr;
                    orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
                }
            }
            break;
        case BookState::LANDED:
            state = BookState::OPENING;
            break;
        case BookState::OPENING:
            openAngle += openSpeed * deltaTime;
            if (openAngle >= maxOpenAngle) {
                openAngle = maxOpenAngle;
                state = BookState::OPENED;
            }
            break;
        case BookState::OPENED:
            break; // Remains open
        case BookState::ON_SHELF:
        default:
            break; // Do nothing
        }
    }
};

class Collectible {
public:
    AssimpModel* model;
    glm::vec3 position;
    float scale;
    glm::vec3 AABBmin;
    glm::vec3 AABBmax;
    bool collected;
    glm::vec3 color;
    OrbState state = OrbState::SPAWNING;
    glm::vec3 spawnPosition;
    glm::vec3 idlePosition;
    float levitationHeight = 0.6f;
    float levitationStartTime = 0.0f;
    float levitationDuration = 0.75f;

    Collectible(AssimpModel* mdl, const glm::vec3& spawnPos, float scl, const glm::vec3& clr)
        : model(mdl), position(spawnPos), scale(scl), collected(false), color(clr),
        state(OrbState::LEVITATING), spawnPosition(spawnPos)
    {
        idlePosition = spawnPosition + glm::vec3(0.0f, levitationHeight, 0.0f);
        levitationStartTime = glfwGetTime();
        updateAABB();
    }

    void updateAABB() {
        if (!model) return;
        glm::vec3 localMin = model->getBoundingBoxMin() * scale;
        glm::vec3 localMax = model->getBoundingBoxMax() * scale;
        AABBmin = localMin + position;
        AABBmax = localMax + position;
    }

    void updateLevitation(float currentTime) {
        if (state == OrbState::LEVITATING) {
            float elapsedTime = currentTime - levitationStartTime;
            float t = glm::clamp(elapsedTime / levitationDuration, 0.0f, 1.0f);
            t = t * t * (3.0f - 2.0f * t); // Smoothstep
            position = glm::mix(spawnPosition, idlePosition, t);
            updateAABB();
            if (t >= 1.0f) {
                state = OrbState::IDLE;
                position = idlePosition;
                updateAABB();
            }
        }
    }
};