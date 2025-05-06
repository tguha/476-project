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

    std::cout << "Enemy Position: " << this->getPosition().x << ", " << this->getPosition().z << std::endl;
    std::cout << "Enemy Grid Position: " << enemyX << ", " << enemyY << std::endl;
    

    glm::ivec2 start = enemyPos;
    glm::ivec2 goal = playerPos;

    std::cout << "1" << std::endl;

    // Define the cost function for pathfinding
    auto costFunc = [&](Pathfinder::Node* from, Pathfinder::Node* to) -> Pathfinder::PathCost {
        if (!grid.inBounds(to->position)) {
            return { false, std::numeric_limits<float>::infinity() };
        }

        const auto& cell = grid.getCell(to->position);

        // Block movement if cell has an object (you can refine this logic)
        if (cell.objectType != LibraryGen::CellObjType::NONE) {
            return { false, std::numeric_limits<float>::infinity() };
        }

        return { true, 1.0f }; // Constant cost for now
    };

    // Reset and find path
    pathfinder.resetNodes();

    std::cout << "2" << std::endl;
    // pathfinder.setStart(start);
    std::vector<glm::ivec2> path = pathfinder.findPath(start, goal, costFunc);

    std::cout << "3" << std::endl;

    // Move to the next step
    if (path.size() > 1) {
        glm::ivec2 nextPos = path[1]; // path[0] is current position
        
        float worldX = grid.mapGridXtoWorldX(nextPos.x);
        float worldZ = grid.mapGridYtoWorldZ(nextPos.y);

        std::cout << "Next Position: " << worldX << ", " << worldZ << std::endl;
        glm::vec3 nextPositionVec3(worldX, this->getPosition().y, worldZ);
        glm::vec3 direction = glm::normalize(nextPositionVec3 - this->getPosition());
        // this->setPosition(nextPositionVec3);
        this->move(direction, deltaTime); // Move towards the next position
    }
}



// --- Implement any overridden virtual functions here ---
// void Enemy::move(const glm::vec3& direction) {
//     // Enemy-specific movement logic
//     Entity::move(direction); // Optionally call base class logic
// }

