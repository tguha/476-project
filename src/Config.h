#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <string>
#include <random>

// It's often better to use static const or constexpr for typed constants
// instead of #define for better type safety and namespacing

namespace Config {
    // --- Global Game Settings ---

    // Rendering & Shaders
    constexpr int NUM_LIGHTS = 4;
    constexpr int MAX_BONES = 200;

    // UI
    constexpr bool SHOW_HEALTHBAR = true;
	constexpr bool SHOW_MINIMAP = true;

    const std::string RESOURCE_DIRECTORY_PREFIX = "../resources"; // Default, can be overridden

    // Default Window Dimensions
    constexpr int DEFAULT_WINDOW_WIDTH = 640;
    constexpr int DEFAULT_WINDOW_HEIGHT = 480;

    // Player settings
    constexpr int INVENTORY_SIZE = 8;
    constexpr float PLAYER_HP_MAX = 100.0f;
    constexpr float PLAYER_HP_MIN = 0.0f;
    constexpr float PLAYER_MOVE_SPEED = 0.045f;
    constexpr float PLAYER_HIT_DURATION = 0.5f; // Duration of hit effect

    // Enemy settings
    constexpr int NUM_ENEMIES = 3;
    // constexpr float ENEMY_HP_MAX = 200.0f;
    // constexpr float ENEMY_HP_MIN = 0.0f;
    // constexpr float ENEMY_MOVE_SPEED = 0.02f;
    constexpr float ENEMY_ATTACK_DAMAGE = 10.0f;
    constexpr float ENEMY_HIT_DURATION = 0.5f; // Duration of hit effect
    constexpr float ENEMY_HIT_COOLDOWN = 1.0f; // Cooldown before enemy can be hit again

    // Ice Elemental settings
    constexpr float ICE_ELEMENTAL_HP_MAX = 200.0f;
    constexpr float ICE_ELEMENTAL_TRANS_Y = 2.0f;
    constexpr float ICE_ELEMENTAL_MOVE_SPEED = 0.02f;
    constexpr float ICE_ELEMENTAL_ROTATION_SPEED = 1.0f; // Radians per second
    constexpr float ICE_ELEMENTAL_MELEE_DAMAGE = 20.0f;
    constexpr float ICE_ELEMENTAL_MELEE_SPEED = 1.0f;
    constexpr float ICE_ELEMENTAL_MELEE_RANGE = 3.0f;

    // Projectile settings
    constexpr float PROJECTILE_DAMAGE = 25.0f;

    // Camera settings
    constexpr float CAMERA_DEFAULT_RADIUS = 5.0f;
    constexpr float CAMERA_DEFAULT_THETA_DEGREES = 0.0f;
    constexpr float CAMERA_DEFAULT_PHI_DEGREES = -30.0f;
    constexpr float CAMERA_MOUSE_SENSITIVITY = 0.005f;
    constexpr float CAMERA_SCROLL_SENSITIVITY_DEGREES = 1.3f;
    constexpr float CAMERA_PHI_MIN_DEGREES = -80.0f;
    constexpr float CAMERA_PHI_MAX_DEGREES = -10.0f;

    // Gameplay
    constexpr float GROUND_HEIGHT = 0.0f;
    constexpr float INTERACTION_RADIUS = 5.0f;
    constexpr float SPELL_PROJECTILE_SPEED = 20.0f;
    constexpr float SPELL_PROJECTILE_LIFETIME = 2.0f;
    constexpr glm::vec3 SPELL_PROJECTILE_SCALE = glm::vec3(0.05f, 0.05f, 0.6f);
    constexpr float SPELL_DAMAGE_AMOUNT = 25.0f;

    // --- Utility Functions ---

    // A random float generator
    inline float randFloat(float l, float h) {
        static std::mt19937 generator(std::random_device{}());
        std::uniform_real_distribution<float> distribution(l, h);
        return distribution(generator);
    }
}

