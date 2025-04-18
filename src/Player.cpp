#include "Player.h"

Player::Player(const glm::vec3& position, float hitpoints, float moveSpeed)
    : Entity(position, hitpoints, moveSpeed) {
    }

SpellSlot Player::getCurrentSpellSlot() {
    return currentSpellSlot;
}

void Player::equipSpell(Spell *spell, int spellSlot) {
    if (spellSlot == SpellSlot::SLOT_ONE) {
        spellInventory[0] = *spell;
    } else if (spellSlot == SpellSlot::SLOT_TWO) {
        spellInventory[1] = *spell;
    }
}

void initPlayer() {}