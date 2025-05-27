#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <string>
#include <random>

// It's often better to use static const or constexpr for typed constants
// instead of #define for better type safety and namespacing

namespace Config {// --- Global Game Settings (Config::<thing you want>)---

    const std::string RESOURCE_DIRECTORY_PREFIX = "../resources"; // Default, can be overridden

    // Debugging --- set all to false for release builds
    constexpr bool DEBUG_ENEMY_MOVEMENT = true;
    constexpr bool DEBUG_ROOM_PLACEMENT = false;
    constexpr bool DEBUG_TEX_LOADING = false;
    constexpr bool DEBUG_PLAYER_AABB = false;
    constexpr bool DEBUG_ORB_PICKUP = false;
    constexpr bool DEBUG_PLAYER_HP = false;
    constexpr bool DEBUG_SHADER = true; // Sets verbose flag for shaders
    constexpr bool DEBUG_SHADER_PARAMS = false; // Debug lighting settings

    // In-Game Debugging (can be toggled real time)
    inline static bool DEBUG_LIGHTING = false;
    inline static bool DEBUG_GEOM = false;

    // Rendering & Shaders
    constexpr int MAX_BONES = 200;
    constexpr float ORTHO_SIZE = 60.0f;
    inline static bool SHADOW = true;
    inline static float EXPOSURE = 0.5f; // Default exposure
    inline static float SATURATION = 2.7f; // Default saturation
	inline static vec3 LIGHT_COLOR = vec3(1.0f, 1.0f, 1.0f); // Default light color

    // Game Elements --- should be set to true to enable drawing of the system
    constexpr bool DRAW_PARTICLES = true;
    constexpr bool DRAW_HEALTHBAR = true;
	constexpr bool DRAW_MINIMAP = true;
    constexpr bool DRAW_PLAYER_DAMAGE = true;
    constexpr bool DRAW_PAW_PRINTS = true;

    // Default Window Dimensions
    constexpr int DEFAULT_WINDOW_WIDTH = 1920;
    constexpr int DEFAULT_WINDOW_HEIGHT = 1080;

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
    constexpr float PROJECTILE_DAMAGE = 100.0f;

    // Camera settings
    constexpr float CAMERA_DEFAULT_RADIUS = 5.0f;
    constexpr float CAMERA_DEFAULT_THETA_DEGREES = 0.0f;
    constexpr float CAMERA_DEFAULT_PHI_DEGREES = -30.0f;
    constexpr float CAMERA_MOUSE_SENSITIVITY = 0.005f;
    constexpr float CAMERA_SCROLL_SENSITIVITY_DEGREES = 1.3f;
    constexpr float CAMERA_PHI_MIN_DEGREES = -80.0f;
    constexpr float CAMERA_PHI_MAX_DEGREES = -10.0f;

    // Gameplay
    constexpr float INTERACTION_RADIUS = 5.0f;
    constexpr float SPELL_PROJECTILE_SPEED = 20.0f;
    constexpr float SPELL_PROJECTILE_LIFETIME = 2.0f;
    constexpr glm::vec3 SPELL_PROJECTILE_SCALE = glm::vec3(0.05f, 0.05f, 0.6f);
    constexpr float SPELL_DAMAGE_AMOUNT = 25.0f;

    // Scene
    constexpr float GROUND_SIZE = 20.0f;
    constexpr float GROUND_HEIGHT = 0.0f;

    // Rotation Constants
    constexpr float HALF_PI = glm::half_pi<float>();
    constexpr float PI = glm::pi<float>();

    // Paw Print settings
    constexpr int PRINTS_MAX = 16;
    constexpr float PRINTS_LIFETIME = 5.0f; // seconds

    // --- Utility Functions ---

    // A random float generator
    inline float randFloat(float l, float h) {
        static std::mt19937 generator(std::random_device{}());
        std::uniform_real_distribution<float> distribution(l, h);
        return distribution(generator);
    }
}
