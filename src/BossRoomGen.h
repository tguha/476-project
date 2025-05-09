#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "Grid.h"
#include <random>
#include <iostream>
#include <map>

class BossRoomGen {
    public:
        BossRoomGen() : grid(glm::ivec2(1, 1), Cell(CellType::NONE)) {}

        // enum CellType {NONE, SHELF, PATH, SPAWN, BOSS_ENTRANCE, TOP_BORDER, BOTTOM_BORDER, LEFT_BORDER, RIGHT_BORDER};

        enum class CellType {
            NONE, // Empty space
            CLUSTER,
            ENTRANCE,
            BORDER,
            EXIT,
        };

        enum class ClusterType {
            NONE,
        };

        enum class BorderType {
            NONE,
            CIRCULAR_BORDER,
            ENTRANCE_MIDDLE,
            ENTRANCE_SIDE,
            EXIT_MIDDLE,
            EXIT_SIDE,
        };

        enum class CellObjType {
            NONE,
        };

        struct transform {
            glm::vec3 position;
            float rotation; // Rotation in degrees
            glm::vec3 scale;
        };

        struct Cell {
            CellType type;
            // union {
            //     ClusterType clusterType; // if type == CLUSTER
            //     BorderType borderType; // if type == BORDER
            // };
            ClusterType clusterType = ClusterType::NONE; // Default to NONE
            BorderType borderType = BorderType::NONE; // Default to NONE
            CellObjType objectType = CellObjType::NONE; // Default to NONE
            transform transformData = {glm::vec3(0), glm::radians(0.0f), glm::vec3(1)}; // Default transform data

            Cell() : type(CellType::NONE) {} // Default constructor
            Cell(CellType t) : type(t) {} // Constructor with type
            Cell(CellType t, ClusterType ct) : type(t), clusterType(ct) {} // Constructor with type and cluster type
            Cell(CellType t, BorderType bt) : type(t), borderType(bt) {} // Constructor with type and border type
            Cell(CellType t, ClusterType ct, CellObjType ot) : type(t), clusterType(ct), objectType(ot) {} // Constructor with type, cluster type and object type
            Cell(CellType t, CellObjType ot) : type(t), objectType(ot) {} // Constructor with type and object type
        };


        // };

        void generate(glm::ivec2 bossGridSize, glm::ivec2 libraryGridSize, glm::vec3 libraryOrigin, glm::ivec2 librarybossEntrDir);
        const Grid<Cell>& getGrid() const { return grid; }
        // Cell getCell(const glm::ivec2& pos) const { return grid.getCell(pos); }
        std::mt19937& getSeedGen() { return seedGen; }
        const glm::vec3& getWorldOrigin() const { return BossroomworldOrigin; } // Get the world origin for the grid

        int mapXtoGridX(float x) const {
            // float worldXwidth = size.x;
            // return (x-(-(size.x))/worldXwidth) * size.x;
            float worldXWidth = grid.getSize().x * 2;
            float localX = x - BossroomworldOrigin.x;
            // return (x - (-size.x)) /  (worldXWidth / (size.x - 1));
            return static_cast<int>((localX - (-grid.getSize().x)) / (worldXWidth / (grid.getSize().x - 1)));
        }

        int mapZtoGridY(float z) const {
            // float worldZwidth = size.y;
            // return (z-(-(size.y))/worldZwidth) * size.y;
            float worldZwidth = grid.getSize().y * 2;
            // return (z - (-size.y)) / (worldZwidth / (size.y - 1));
            float localZ = z - BossroomworldOrigin.z;
            return static_cast<int>((localZ - (-grid.getSize().y)) / (worldZwidth / (grid.getSize().y - 1)));
        }

        float mapGridXtoWorldX(int x) const {
            float worldXwidth = grid.getSize().x * 2;
            // return (x * (worldXwidth / (size.x - 1))) - size.x;
            float localX = (-grid.getSize().x) + (x * (worldXwidth / (grid.getSize().x - 1)));
            return BossroomworldOrigin.x + localX;
        }

        float mapGridYtoWorldZ(int y) const {
            float worldZwidth = grid.getSize().y * 2;
            // return (y * (worldZwidth / (size.y - 1))) - size.y;
            float localZ = (-grid.getSize().y) + (y * (worldZwidth / (grid.getSize().y - 1)));
            return BossroomworldOrigin.z + localZ;
        }

    private:
        Grid<Cell> grid; // Grid of CellType
        std::vector<glm::vec2> EntranceCenters;
        std::vector<glm::vec2> ExitCenters;
        std::mt19937 seedGen;
        glm::vec2 spawnPosinGrid;
        glm::vec2 bossEntranceDir;
        std::vector<glm::vec2> avoidPoints;
        glm::ivec2 gridSize;
        glm::vec3 BossroomworldOrigin = glm::vec3(0, 0, 0); // World origin for the grid


        std::map<ClusterType, float> objMinSpacing = {
        };

        std::map<ClusterType, int> MaxobjAmount = {
        };

        std::map<ClusterType, int> objAmount = {
        };

        std::vector<ClusterType> clusterOptions = {
        };

        std::vector<std::pair<glm::ivec2, glm::ivec2>> selectedEdges;

        void placeBorder();
        void placeEntrance(); // Place the entrance in the boss room
        void placeExit(); // Place the exit in the boss room
};