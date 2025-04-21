#ifndef LIBRARYGEN_H
#define LIBRARYGEN_H

#include <vector>
#include <glm/glm.hpp>
#include "Delaunay2D.h"
#include "Grid.h"
#include "Pathfinder.h"
#include <random>
#include <iostream>

class LibraryGen {
    public:
        LibraryGen() : grid(glm::ivec2(1, 1), glm::ivec2(0, 0), NONE) {}

        enum CellType {NONE, SHELF, PATH};

        void generate(glm::ivec2 size, glm::ivec2 offset = {0, 0});
        const Grid<CellType>& getGrid() const { return grid; }
        CellType getCell(const glm::ivec2& pos) const { return grid.getCell(pos); }

    private:
        Grid<CellType> grid; // Grid of CellType
        std::vector<glm::vec2> clusterCenters;
        std::mt19937 seedGen;

        std::vector<std::pair<glm::ivec2, glm::ivec2>> selectedEdges;

        void placeClusters(int count);
        void triangulateClusters();
        void generatePaths();
        void addShelfWalls();

        Pathfinder::PathCost calcCost(const glm::ivec2& from, const glm::ivec2& to);
};

#endif // LIBRARYGEN_H