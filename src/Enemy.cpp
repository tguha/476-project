#include "Enemy.h"
#include "Grid.h"

// --- Constructor Implementation ---
Enemy::Enemy(const glm::vec3& position, float hitpoints, float moveSpeed, AssimpModel* model, const glm::vec3& scale, const glm::vec3& rotation)
            : Entity(position, hitpoints, moveSpeed, model, scale, rotation) {}


bool Enemy::isHit() const {
    return this->hit;
}

void Enemy::setHit(bool hit) {
    this->hit = hit;
}

void Enemy::moveTowardsPlayer(Grid<LibraryGen::Cell>& grid, Pathfinder& pathfinder, const glm::vec3& playerPosition, float deltaTime) {
    int playerX = grid.mapXtoGridX(playerPosition.x);
    int playerY = grid.mapZtoGridY(playerPosition.z);
    glm::ivec2 playerPos = glm::ivec2(playerX, playerY);

    int enemyX = grid.mapXtoGridX(this->getPosition().x);
    int enemyY = grid.mapZtoGridY(this->getPosition().z);
    glm::ivec2 enemyPos = glm::ivec2(enemyX, enemyY);
    
    glm::ivec2 start = enemyPos;
    glm::ivec2 goal = playerPos;

    auto costFunc = [&](Pathfinder::Node* from, Pathfinder::Node* to) -> Pathfinder::PathCost {
        if (!grid.inBounds(to->position)) {
            return { false, std::numeric_limits<float>::infinity() };
        }

        const auto& cell = grid.getCell(to->position);

        if (cell.objectType != LibraryGen::CellObjType::NONE) {
            return { false, std::numeric_limits<float>::infinity() };
        }

        return { true, 1.0f }; // Constant cost
    };

    pathfinder.resetNodes();

    std::vector<glm::ivec2> path = pathfinder.findPath(start, goal, costFunc);

    // Move to the next step
    if (path.size() > 1) {
        glm::ivec2 nextPos = path[1]; // path[0] is current position
        
        float worldX = grid.mapGridXtoWorldX(nextPos.x);
        float worldZ = grid.mapGridYtoWorldZ(nextPos.y);
        std::cout << "Next Grid Position: " << nextPos.x << ", " << nextPos.y << std::endl;
        std::cout << "Next World Position: " << worldX << ", " << worldZ << std::endl;
        glm::vec3 nextPositionVec3(worldX, this->getPosition().y, worldZ);
        glm::vec3 direction = glm::normalize(nextPositionVec3 - this->getPosition());

        this->move(direction, deltaTime);
    }
}

void Enemy::attack(float damage, float deltaTime) {
    // Implement attack logic here
    std::cout << "Enemy attacks with damage: " << damage << std::endl;

}
