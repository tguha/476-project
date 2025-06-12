#include "Enemy.h"
#include "Spell.h"
#include "Config.h"

#define BOSS_HP_MAX 600.0f
#define BOSS_ENRAGED_HP 250.0f
#define BOSS_SPECIAL_ATTACK_COOLDOWN 5.0f

class BossEnemy : public Enemy {
    private:

        bool enraged;
        float specialAttackCooldown;
        float attack1Cooldown = 0.0f; // Cooldown for the first attack
        glm::vec3 bossDirection = glm::vec3(0.0f, 0.0f, 0.0f);
        SpellType BossSpellType = SpellType::NONE;
        float slamCooldown = Config::BOSS_SLAM_COOLDOWN;
        float slamDuration = Config::BOSS_SLAM_DURATION;
        float bossDeathTimer = 0.0f; // Timer for boss death animation
        float bossDeathDuration = 3.0f; // Duration of the death animation

    public:
        BossEnemy(const glm::vec3& position, float hitpoints, AssimpModel* model, const glm::vec3& scale, const glm::vec3& rotation, float specialAttackCooldown, SpellType spellType);

        enum class BossPhase {
            PHASE_1,
            PHASE_2,
            PHASE_3
        } phase = BossPhase::PHASE_1;

        void changePhase();
        void specialAttack(float damage, float deltaTime);
        void lookAtPlayer(const glm::vec3& playerPosition);
        void launchProjectile(const glm::vec3& targetPosition, float speed, float damage, float deltaTime);
        BossPhase getPhase() const { return phase; }
        bool isEnraged() const { return enraged; }
        glm::vec3 getBossDirection() const { return bossDirection; }
        float getSpecialAttackCooldown() const { return specialAttackCooldown; }
        void setSpecialAttackCooldown(float cooldown) { specialAttackCooldown = cooldown; }
        float getAttack1Cooldown() const { return attack1Cooldown; }
        void setAttack1Cooldown(float cooldown) { attack1Cooldown = cooldown; }
        float getSlamCooldown() const { return slamCooldown; }
        float getSlamDuration() const { return slamDuration; }
        void setSlamCooldown(float cooldown) { slamCooldown = cooldown; }
        void setSlamDuration(float duration) { slamDuration = duration; }
        SpellType getBossSpellType() const { return BossSpellType; }
        void resetPhase() {
            phase = BossPhase::PHASE_1;
            enraged = false;
            specialAttackCooldown = BOSS_SPECIAL_ATTACK_COOLDOWN;
            attack1Cooldown = 0.0f;
        }
        void setBossDeathTimer(float timer) { bossDeathTimer = timer; }
        float getBossDeathTimer() const { return bossDeathTimer; }
        void setBossDeathDuration(float duration) { bossDeathDuration = duration; }
        float getBossDeathDuration() const { return bossDeathDuration; }
        void setBossSpellType(SpellType spellType) { BossSpellType = spellType; }
};