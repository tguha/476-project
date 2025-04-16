#ifndef PLAYER_H
#define PLAYER_H

#include "Entity.h"
#include "Spell.h"

typedef enum {
    SLOT_ONE,
    SLOT_TWO
} SpellSlot;

class Player : public Entity {
    private:
        Spell *currentSpell;
        Spell spellInventory[MAX_SPELLS] = {
            Spell(NO_SPELL, 0, 0, 0, 0, NO_SPELL),
            Spell(NO_SPELL, 0, 0, 0, 0, NO_SPELL)
        };

        SpellSlot currentSpellSlot;

    public:
        Player(AssimpModel* model, Texture *texture, const glm::vec3& position, float hitpoints, float moveSpeed);

        SpellSlot getCurrentSpellSlot();
        void setCurrentSpellSlot(SpellSlot slot);
        void equipSpell(Spell *spell, int spellSlot);
        void castSpell();
};

void initPlayer();

#endif // PLAYER_H