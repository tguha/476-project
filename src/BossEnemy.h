#include "Enemy.h"
#include "Spell.h"

#define BOSS_HP_MAX 500.0f
#define BOSS_ENRAGED_HP 250.0f
#define BOSS_SPECIAL_ATTACK_COOLDOWN 5.0f

class BossEnemy : public Enemy {
    private:
        enum class BossPhase {
            PHASE_1,
            PHASE_2,
            PHASE_3
        } phase = BossPhase::PHASE_1;
        bool enraged;
        float specialAttackCooldown;
        glm::vec3 bossDirection = glm::vec3(0.0f, 0.0f, 0.0f);
        SpellType BossSpellType = SpellType::NONE;

    public:
        BossEnemy(const glm::vec3& position, float hitpoints, AssimpModel* model, const glm::vec3& scale, const glm::vec3& rotation, float specialAttackCooldown, SpellType spellType);

        void changePhase();
        void specialAttack(float damage, float deltaTime);
        void lookAtPlayer(const glm::vec3& playerPosition);
        void launchProjectile(const glm::vec3& targetPosition, float speed, float damage, float deltaTime);
        BossPhase getPhase() const { return phase; }
        bool isEnraged() const { return enraged; }
        glm::vec3 getBossDirection() const { return bossDirection; }
        float getSpecialAttackCooldown() const { return specialAttackCooldown; }
        void setSpecialAttackCooldown(float cooldown) { specialAttackCooldown = cooldown; }
        SpellType getBossSpellType() const { return BossSpellType; }
};