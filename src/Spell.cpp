#include "Spell.h"

Spell::Spell(std::string name, float damage, float manaCost, float cooldown, float fireRate, float range, SpellType weakness)
    : name(name), damage(damage), manaCost(manaCost), cooldown(cooldown), fireRate(fireRate), range(range), weakness(weakness) {}

void Spell::cast(glm::vec3 playerPosition, glm::vec3 targetDirection) {
    // Implementation for casting the spell will go here
    // For now, it's empty as in the original Spell.cpp, but with correct signature
}

float Spell::getDamage() { return damage; }
float Spell::getManaCost() { return manaCost; }
float Spell::getCooldown() { return cooldown; }
float Spell::getFireRate() { return fireRate; }
float Spell::getRange() { return range; }
SpellType Spell::getWeakness() { return weakness; }

std::string Spell::getName() { return name; }

float Spell::getDamage() { return damage; }
float Spell::getManaCost() { return manaCost; }
float Spell::getCooldown() { return cooldown; }
float Spell::getFireRate() { return fireRate; }
float Spell::getRange() { return range; }
SpellType Spell::getWeakness() { return weakness; }

std::string Spell::getName() { return name; }

void initSpells() {

}