#include "Player.h"

Player::Player(const glm::vec3& position, float hitpoints, float moveSpeed, AssimpModel* model, const glm::vec3& scale, const glm::vec3& rotation)
    : Entity(position, hitpoints, moveSpeed, model, scale, rotation) {
    // Add any Player-specific initialization here
    std::cout << "Player Entity Created." << std::endl;
}

SpellSlot Player::getActiveSpellSlot() {
    return activeSpellSlot;
}

void Player::pickupSpell(Spell *spell, SpellSlot activeSpellSlot) {
    
    // Put spell in inventory
    if (isInventoryFull()) {
        for (int i = 0; i < Config::INVENTORY_SIZE; i++) {
            if (spellInventory[i].getType() == SpellType::NO_SPELL) {
                spellInventory[i] = *spell;
                return;
            }
        }
    } else {
        // If inventory is full, replace the spell in the active slot
        /* Logic to drop the spell in the active slot ?? */
        spellInventory[activeSpellSlot] = *spell;
    }
}

bool Player::isInventoryFull() {
    for (int i = 0; i < Config::INVENTORY_SIZE; i++) {
        if (spellInventory[i].getType() == SpellType::NO_SPELL) {
            return false;
        }
    }
    return true;
}

Spell Player::getSpellFromSpellSlot(SpellSlot slot) {
    return spellInventory[slot];
}

void Player::setSprintFlag(bool flag) {
    sprintFlag = flag;
}

void Player::move(const glm::vec3& direction, float deltaTime) {
    if (sprintFlag) {
        Entity::move(direction * 2.0f, deltaTime);     // arbitrary sprint speed
    } else {
        Entity::move(direction, deltaTime);
    }
}

void Player::castSpell() {
    if (currentSpell->getType() != SpellType::NO_SPELL) {
        currentSpell->cast();
    }
}

void initPlayer() {}