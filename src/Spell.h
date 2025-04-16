#ifndef SPELL_H
#define SPELL_H

#define MAX_SPELLS 2

typedef enum {
    NO_SPELL,
    FIRE,
    ICE,
    LIGHTNING
} SpellType;

class Spell {
    private:
        SpellType type;
        float damage;
        float fireRate;
        float range;
        float cooldown;
        SpellType weakness;

    public:
        Spell(SpellType type, float damage, float fireRate, float range, float cooldown, SpellType weakness)
            : type(type), damage(damage), fireRate(fireRate), range(range), cooldown(cooldown), weakness(weakness) {}
        void cast();
};

void initSpells();

#endif // SPELL_H