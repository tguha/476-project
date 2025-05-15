#pragma once

#include <iostream>
#include <string>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include "../src/GameObjectTypes.h"

using namespace std;

class Spell {
private:
    std::string name;
    float damage;
    float manaCost;
    float cooldown;
    float fireRate;
    float range;
    SpellType weakness;

public:
    Spell(std::string name, float damage, float manaCost, float cooldown, float fireRate, float range, SpellType weakness);
    void cast(glm::vec3 playerPosition, glm::vec3 targetDirection);
    SpellType getType();
    float getDamage();
    float getManaCost();
    float getCooldown();
    float getFireRate();
    float getRange();
    SpellType getWeakness();
    std::string getName();
};

void initSpells();