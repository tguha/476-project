#include "BossRoomGen.h"

void BossRoomGen::generate(glm::ivec2 bossGridSize, glm::ivec2 libraryGridSize, glm::vec3 libraryOrigin, glm::ivec2 librarybossEntrDir) {
    grid = Grid<Cell>(bossGridSize, Cell(CellType::NONE));
    this->gridSize = bossGridSize;

    this->bossEntranceDir = librarybossEntrDir; // Set the boss entrance direction

    // Calculate the world origin based on the entrance direction and the librarygen parameters
    if (librarybossEntrDir.x > 0) {
        BossroomworldOrigin = libraryOrigin + glm::vec3(libraryGridSize.x + bossGridSize.x, 0, 0);
    } else if (librarybossEntrDir.x < 0) {
        BossroomworldOrigin = libraryOrigin + glm::vec3(-libraryGridSize.x - bossGridSize.x, 0, 0);
    } else if (librarybossEntrDir.y > 0) {
        BossroomworldOrigin = libraryOrigin + glm::vec3(0, 0, libraryGridSize.y + bossGridSize.y);
    } else if (librarybossEntrDir.y < 0) {
        BossroomworldOrigin = libraryOrigin + glm::vec3(0, 0, -libraryGridSize.y - bossGridSize.y);
    }

    int x = mapXtoGridX(BossroomworldOrigin.x);
    int y = mapZtoGridY(BossroomworldOrigin.z);
    grid[glm::ivec2(x, y)] = Cell(CellType::BOSSSPAWN); // Mark the spawn position in the grid

    std::cout << "Boss room world origin: " << BossroomworldOrigin.x << ", " << BossroomworldOrigin.y << ", " << BossroomworldOrigin.z << std::endl;
    placeBorder();
    placeEntrance(); // Place the entrance in the boss room
    placeExit();
    placeClusters(1);
}

void BossRoomGen::placeBorder() {
    std::cout << "[BossRoomGen] Placing circular border..." << std::endl;

    const glm::ivec2 size = this->gridSize; // Size of the grid
    const glm::vec2 center = glm::vec2(size.x / 2.0f, size.y / 2.0f);

    float radiusX = size.x * 0.35f; // Ellipse width
    float radiusY = size.y * 0.35f; // Ellipse height

    this->radiusX = radiusX; // Store the radius for later use
    this->radiusY = radiusY; // Store the radius for later use

    float entranceWidth = 3.0f; // Width of the opening at the bottom (in grid units)
    this->entranceWidth = entranceWidth; // Store the entrance width for later use

    for (int y = 0; y < size.y; ++y) {
        for (int x = 0; x < size.x; ++x) {
            glm::vec2 pos = glm::vec2(x, y);
            glm::vec2 delta = pos - center;

            // Normalized ellipse equation: (x/rx)^2 + (y/ry)^2 = 1
            float ellipseValue = (delta.x * delta.x) / (radiusX * radiusX) +
                                 (delta.y * delta.y) / (radiusY * radiusY);

            // Just inside the ellipse = interior space
            // Just outside the ellipse = border
            if (ellipseValue >= 0.95f && ellipseValue <= 1.15f) {
                // Check for entrance gap (bottom center)

                if (bossEntranceDir.y > 0) {
                    if ((y < center.y && std::abs(x - center.x) < entranceWidth / 2.0f)) {
                        grid[glm::ivec2(x, y)] = Cell(CellType::ENTRANCE); // Leave opening
                        EntranceCenters.push_back(glm::vec2(x, y));
                        continue; // Leave opening
                    } else if ((y > center.y && std::abs(x - center.x) < entranceWidth / 2.0f)) {
                        grid[glm::ivec2(x, y)] = Cell(CellType::EXIT); // Leave opening
                        ExitCenters.push_back(glm::vec2(x, y));
                        continue; // Leave opening
                    }
                } else if (bossEntranceDir.y < 0) {
                    if ((y > center.y && std::abs(x - center.x) < entranceWidth / 2.0f)) {
                        grid[glm::ivec2(x, y)] = Cell(CellType::ENTRANCE); // Leave opening
                        EntranceCenters.push_back(glm::vec2(x, y));
                        continue; // Leave opening
                    } else if ((y < center.y && std::abs(x - center.x) < entranceWidth / 2.0f)) {
                        grid[glm::ivec2(x, y)] = Cell(CellType::EXIT); // Leave opening
                        ExitCenters.push_back(glm::vec2(x, y));
                        continue; // Leave opening
                    }
                } else if (bossEntranceDir.x > 0) {
                    if ((x < center.x && std::abs(y - center.y) < entranceWidth / 2.0f)) {
                        grid[glm::ivec2(x, y)] = Cell(CellType::ENTRANCE); // Leave opening
                        EntranceCenters.push_back(glm::vec2(x, y));
                        continue; // Leave opening
                    } else if ((x > center.x && std::abs(y - center.y) < entranceWidth / 2.0f)) {
                        grid[glm::ivec2(x, y)] = Cell(CellType::EXIT); // Leave opening
                        ExitCenters.push_back(glm::vec2(x, y));
                        continue; // Leave opening
                    }
                } else if (bossEntranceDir.x < 0) {
                    if ((x > center.x && std::abs(y - center.y) < entranceWidth / 2.0f)) {
                        grid[glm::ivec2(x, y)] = Cell(CellType::ENTRANCE); // Leave opening
                        EntranceCenters.push_back(glm::vec2(x, y));
                        continue; // Leave opening
                    } else if ((x < center.x && std::abs(y - center.y) < entranceWidth / 2.0f)) {
                        grid[glm::ivec2(x, y)] = Cell(CellType::EXIT); // Leave opening
                        ExitCenters.push_back(glm::vec2(x, y));
                        continue; // Leave opening
                    }
                }

                Cell borderCell(CellType::BORDER, BorderType::CIRCULAR_BORDER); // Create a border cell

                glm::vec2 tangent(-delta.y, delta.x); // Tangent vector (perpendicular to delta)
                float angleRadians = atan2(tangent.y, tangent.x); // Angle in radians
                float angleDegrees = glm::degrees(angleRadians); // Convert to degrees

                borderCell.transformData.rotation = -angleDegrees; // Adjust rotation to match the tangent direction

                // Place border cell
                std::cout << "Placing border at: " << x << ", " << y << std::endl;
                grid[glm::ivec2(x, y)] = borderCell; // Set the cell in the grid
            }
        }
    }

    std::cout << "[BossRoomGen] Circular border placed.\n";
}

void BossRoomGen::placeEntrance() {
    // Place the entrance in the boss room
    std::cout << "[BossRoomGen] Placing entrance..." << std::endl;
    const glm::ivec2 size = this->gridSize; // Size of the grid

    for (const auto& center : EntranceCenters) {
        grid[glm::ivec2(center)] = Cell(CellType::ENTRANCE); // Mark the entrance position in the grid
        if (center.x == (size.x / 2) || center.y == (size.y / 2)) {
            grid[glm::ivec2(center)].borderType = BorderType::ENTRANCE_MIDDLE;
        } else {
            grid[glm::ivec2(center)].borderType = BorderType::ENTRANCE_SIDE;
            if (bossEntranceDir.y > 0) {
                for (int i = center.y; i >= 0; --i) {
                    Cell borderCell(CellType::BORDER, BorderType::ENTRANCE_SIDE); // Create a border cell
                    borderCell.transformData.rotation = 90.0f; // in degrees
                    grid[glm::ivec2(center.x, i)] = borderCell; // Set the cell in the grid
                }
            } else if (bossEntranceDir.y < 0) {
                for (int i = center.y; i < size.y; ++i) {
                    Cell borderCell(CellType::BORDER, BorderType::ENTRANCE_SIDE); // Create a border cell
                    borderCell.transformData.rotation = 90.0f; // in degrees
                    grid[glm::ivec2(center.x, i)] = borderCell; // Set the cell in the grid
                }
            } else if (bossEntranceDir.x > 0) {
                for (int i = center.x; i < size.x; ++i) {
                    Cell borderCell(CellType::BORDER, BorderType::ENTRANCE_SIDE); // Create a border cell
                    grid[glm::ivec2(i, center.y)] = borderCell; // Set the cell in the grid
                }
            } else if (bossEntranceDir.x < 0) {
                for (int i = center.x; i >= 0; --i) {
                    Cell borderCell(CellType::BORDER, BorderType::ENTRANCE_SIDE); // Create a border cell
                    grid[glm::ivec2(i, center.y)] = borderCell; // Set the cell in the grid
                }
            }
        }
    }

    std::cout << "[BossRoomGen] Entrance placed.\n";
}

void BossRoomGen::placeExit() {
    // Place the exit in the boss room
    std::cout << "[BossRoomGen] Placing exit..." << std::endl;
    const glm::ivec2 size = this->gridSize; // Size of the grid

    for (const auto& center : ExitCenters) {
        grid[glm::ivec2(center)] = Cell(CellType::EXIT); // Mark the exit position in the grid
        if (center.x == (size.x / 2) || center.y == (size.y / 2)) {
            grid[glm::ivec2(center)].borderType = BorderType::EXIT_MIDDLE;
            grid[glm::ivec2(center)].transformData.rotation = 180.0f; // in degrees
        } else {
            grid[glm::ivec2(center)].borderType = BorderType::EXIT_SIDE;
            if (bossEntranceDir.y > 0) {
                for (int i = center.y; i < size.y; ++i) {
                    Cell borderCell(CellType::BORDER, BorderType::EXIT_SIDE); // Create a border cell
                    borderCell.transformData.rotation = 90.0f; // in degrees
                    grid[glm::ivec2(center.x, i)] = borderCell; // Set the cell in the grid
                }
            } else if (bossEntranceDir.y < 0) {
                for (int i = center.y; i >= 0; --i) {
                    Cell borderCell(CellType::BORDER, BorderType::EXIT_SIDE); // Create a border cell
                    borderCell.transformData.rotation = 90.0f; // in degrees
                    grid[glm::ivec2(center.x, i)] = borderCell; // Set the cell in the grid
                }
            } else if (bossEntranceDir.x > 0) {
                for (int i = center.x; i >= 0; --i) {
                    Cell borderCell(CellType::BORDER, BorderType::EXIT_SIDE); // Create a border cell
                    grid[glm::ivec2(i, center.y)] = borderCell; // Set the cell in the grid
                }
            } else if (bossEntranceDir.x < 0) {
                for (int i = center.x; i < size.x; ++i) {
                    Cell borderCell(CellType::BORDER, BorderType::EXIT_SIDE); // Create a border cell
                    grid[glm::ivec2(i, center.y)] = borderCell;
                }
            }
        }
    }
}

bool BossRoomGen::isInsideBossArea(const glm::ivec2& gridPos) {
    // Check if the position is inside the boss room area
    glm::vec2 pos = glm::vec2(gridPos.x, gridPos.y);
    glm::vec2 center = glm::vec2(gridSize.x / 2.0f, gridSize.y / 2.0f);

    glm::vec2 delta = pos - center;
    float ellipseValue = (delta.x * delta.x) / (radiusX * radiusX) +
                         (delta.y * delta.y) / (radiusY * radiusY);
    return (ellipseValue <= 0.85f); // Inside the ellipse
}

void BossRoomGen::placeClusters(int count) {
    std::uniform_int_distribution<int> distX(0, grid.getSize().x - 1);
    std::uniform_int_distribution<int> distY(0, grid.getSize().y - 1);

    std::uniform_int_distribution<int> clusterTypeDist(0, clusterOptions.size() - 1);

    int attempts = 0;
    int maxAttempts = grid.getSize().x * grid.getSize().y / 5; // Limit attempts to avoid infinite loop

    for (auto& obj : objAmount) {
        obj.second = 0; // Reset object amounts
    }

    while (clusterCenters.size() < count && attempts < maxAttempts) {
        attempts++;
        glm::ivec2 pos{distX(seedGen), distY(seedGen)};
        bool valid = true;

        if (grid[pos].type == CellType::BOSSSPAWN ||
            !isInsideBossArea(pos)) {
                valid = false; // Skip if the position is not inside the circle boss area or is the spawn position
            }

        // Check if the position is already occupied or too close to existing clusters
        for (const auto& center : clusterCenters) {
            float requiredSpacing = objMinSpacing[grid[center].clusterType]; // Get the required spacing for the cluster type
            for (const auto& avoidPoint : avoidPoints) {
                if (glm::distance(glm::vec2(pos), glm::vec2(avoidPoint)) < requiredSpacing) {
                    valid = false;
                    break;
                }
            }
        }

        // if the position is valid, place the cluster
        if (valid) {
            clusterCenters.push_back(pos);
            ClusterType randomClusterType;

            randomClusterType = clusterOptions[clusterTypeDist(seedGen)]; // Randomly select a cluster type

            // Limits the number of objects of each type based on MaxobjAmount
            while (MaxobjAmount.count(randomClusterType) && objAmount[randomClusterType] >= MaxobjAmount[randomClusterType]) {
                randomClusterType = clusterOptions[clusterTypeDist(seedGen)];
            }

            // add if the cluster type is in the map
            if (objAmount.count(randomClusterType)) {
                objAmount[randomClusterType]++;
            }

            if (randomClusterType == ClusterType::SHELF1) {
                if (grid.inBounds(pos)) {
                    BossRoomGen::Cell cell = Cell(CellType::CLUSTER, randomClusterType, CellObjType::GLOWING_SHELF);
                    cell.transformData.position = glm::vec3(mapGridXtoWorldX(pos.x), 0, mapGridYtoWorldZ(pos.y));
                    grid[pos] = cell; // Mark the cluster position in the grid

                }
            }
        }
    }

}


