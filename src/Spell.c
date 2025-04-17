#include "Spell.h"

Spell::Spell(SpellType type, float damage, float fireRate, float range, float cooldown, SpellType weakness)
    : type(type), damage(damage), fireRate(fireRate), range(range), cooldown(cooldown), weakness(weakness) {}

void Spell::cast() {}