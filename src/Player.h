#ifndef PLAYER_H
#define PLAYER_H

#include "Entity.h"
#include "Spell.h"
#include "Config.h"

typedef enum {
    SLOT_LEFT,
    SLOT_RIGHT,
    SLOT_INV_ONE,
    SLOT_INV_TWO,
    SLOT_INV_THREE,
    SLOT_INV_FOUR,
    SLOT_INV_FIVE,
    SLOT_INV_SIX
} SpellSlot;

class Player : public Entity {
    private:
        Spell *currentSpell;

        // Init spell inventory with empty slots
        Spell spellInventory[Config::INVENTORY_SIZE] = {
            Spell("Empty", SpellType::NO_SPELL, 0, 0, 0, 0, SpellType::NO_SPELL),
            Spell("Empty", SpellType::NO_SPELL, 0, 0, 0, 0, SpellType::NO_SPELL),
            Spell("Empty", SpellType::NO_SPELL, 0, 0, 0, 0, SpellType::NO_SPELL),
            Spell("Empty", SpellType::NO_SPELL, 0, 0, 0, 0, SpellType::NO_SPELL),
            Spell("Empty", SpellType::NO_SPELL, 0, 0, 0, 0, SpellType::NO_SPELL),
            Spell("Empty", SpellType::NO_SPELL, 0, 0, 0, 0, SpellType::NO_SPELL),
            Spell("Empty", SpellType::NO_SPELL, 0, 0, 0, 0, SpellType::NO_SPELL),
            Spell("Empty", SpellType::NO_SPELL, 0, 0, 0, 0, SpellType::NO_SPELL)
        };

        SpellSlot activeSpellSlot;
        bool sprintFlag;
        bool dodgeFlag;

    public:
        Player(const glm::vec3& position, float hitpoints, float moveSpeed, AssimpModel* model, const glm::vec3& scale, const glm::vec3& rotation);

        void move(const glm::vec3& direction, float deltaTime) override;
        SpellSlot getActiveSpellSlot();
        void setCurrentSpellSlot(SpellSlot slot);
        Spell getSpellFromSpellSlot(SpellSlot slot);
        void pickupSpell(Spell *spell, SpellSlot activeSpellSlot);
        void castSpell();
        void setSprintFlag(bool flag);
        void dodge();
        bool isInventoryFull();
};

void initPlayer();

#endif // PLAYER_H