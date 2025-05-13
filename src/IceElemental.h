#include "Enemy.h"
#include "Config.h"

class IceElemental : public Enemy {
 
    public:
        IceElemental(const glm::vec3& position, float hitpoints, float moveSpeed, AssimpModel* model, const glm::vec3& scale = glm::vec3(1.0f), const glm::vec3& rotation = glm::vec3(0.0f));

        void moveTowardsPlayer(const glm::vec3& playerPosition, float deltaTime) override;
        void update(Player* player, float deltaTime) override;
};