#ifndef PLAYER_H
#define PLAYER_H

#include "Entity.h"
#include "Spell.h"
#include "Config.h"
#include "GameObjectTypes.h"

class Player : public Entity {
    private:
        Spell *currentSpell;

        // Init spell inventory with empty slots
        Spell spellInventory[Config::INVENTORY_SIZE] = {
            Spell("Empty", 0, 0, 0, 0, 0, SpellType::NONE),
            Spell("Empty", 0, 0, 0, 0, 0, SpellType::NONE),
            Spell("Empty", 0, 0, 0, 0, 0, SpellType::NONE),
            Spell("Empty", 0, 0, 0, 0, 0, SpellType::NONE),
            Spell("Empty", 0, 0, 0, 0, 0, SpellType::NONE),
            Spell("Empty", 0, 0, 0, 0, 0, SpellType::NONE),
            Spell("Empty", 0, 0, 0, 0, 0, SpellType::NONE),
            Spell("Empty", 0, 0, 0, 0, 0, SpellType::NONE)
        };

        SpellSlot activeSpellSlot;
        bool sprintFlag;
        bool dodgeFlag;
        float damageTimer = 0.0f;

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
        void setDamageTimer(float timer);
        float getDamageTimer();

        void takeDamage(float damage) override;
};

void initPlayer();

#endif // PLAYER_H