#include "LibraryGen.h"
#include "Grid.h"

/*
    * LibraryGen.cpp
    * This file contains the implementation of the LibraryGen class, which is responsible for generating a library layout.
    * It uses a grid-based approach to place shelves and paths, and utilizes Delaunay triangulation for pathfinding.
    *
    *
    */

void LibraryGen::generate(glm::ivec2 size, glm::ivec2 offset) {

    std::cout << "Grid size: " << size.x << ", " << size.y << std::endl;


    grid = Grid<CellType>(size, offset, NONE); // Initialize the grid with the given size and offset


    std::cout << "Generating library layout..." << std::endl;

    seedGen.seed(std::random_device()());

    placeClusters(size.x * size.y / 50); // Destiny controls the number of clusters

    std::cout << "Placed " << clusterCenters.size() << " clusters." << std::endl;

    triangulateClusters();

    std::cout << "Triangulated clusters." << std::endl;

    // generatePaths();

    // std::cout << "Generated paths." << std::endl;

    // addShelfWalls();
}

void LibraryGen::placeClusters(int count) {
    // Randomly place cluster centers in the grid
    std::uniform_int_distribution<int> distX(0, grid.getSize().x - 1);
    std::uniform_int_distribution<int> distY(0, grid.getSize().y - 1);

    std::cout << "Placing clusters..." << std::endl;

    while (clusterCenters.size() < count) {
        glm::ivec2 pos{distX(seedGen), distY(seedGen)};

        // Ensure minimum spacing between clusters
        bool valid = true;

        std::cout << "Trying to place cluster at: " << pos.x << ", " << pos.y << std::endl;

        // Check if the position is already occupied or too close to existing clusters
        for (const auto& center : clusterCenters) {
            if (glm::distance(glm::vec2(center), glm::vec2(pos)) < 5.0f) { // Adjust this number for spacing
                valid = false;
                break;
            }
        }

        // Check if the position is within bounds
        if (valid) {
            // if the position is valid, add it to the cluster centers
            clusterCenters.push_back(pos);

            // Place 3x3 cluster around the center
            for (int y = -1; y <= 1; y++) {
                for (int x = -1; x <= 1; x++) {
                    glm::ivec2 shelfPos = pos + glm::ivec2(x, y);
                    if (grid.inBounds(shelfPos)) {
                        grid[shelfPos] = SHELF;
                    }
                }
            }
        }
    }
}

void LibraryGen::generatePaths() {
    // Check if there are enough centers to create paths
    if (clusterCenters.size() <= 1) {
        std::cerr << "Not enough cluster centers to generate paths." << std::endl;
        return;
    }

    std::cout << "Generating paths between clusters..." << std::endl;


    Pathfinder pathfinder;

    // Connect all clusters with paths
    for (size_t i = 1; i < clusterCenters.size(); i++) {
        std::cout << "Finding path from cluster " << i - 1 << " to cluster " << i << std::endl;

        glm::ivec2 start = glm::ivec2(glm::round(clusterCenters[i-1]));
        glm::ivec2 end = glm::ivec2(glm::round(clusterCenters[i]));

        // Ensure the start and end points are in bounds
        if (!grid.inBounds(start) || !grid.inBounds(end)) {
            std::cerr << "Start or end point out of bounds: " << start.x << ", " << start.y << " to " << end.x << ", " << end.y << std::endl;
            continue;
        }


        auto path = pathfinder.findPath(start, end, [this](const glm::ivec2& from, const glm::ivec2& to) {
            return calcCost(from, to);
            }
        );

        if (path.empty()) {
            std::cerr << "No path found from " << start.x << ", " << start.y << " to " << end.x << ", " << end.y << std::endl;
            continue;
        }

        // Mark the path in the grid
        for (const auto& pos : path) {
            if (grid[pos] == NONE) {
                grid[pos] = PATH;
            } else {
                std::cerr << "Path overlaps with existing shelf at: " << pos.x << ", " << pos.y << std::endl;
            }
        }

        std::cout << "Path found with " << path.size() << " steps." << std::endl;
    }
}

void LibraryGen::triangulateClusters() {
    // Create a Delaunay triangulation of the cluster centers
    std::vector<Delaunay2D::DelaunayVertex> delaunayVertices;


    if (clusterCenters.empty()) {
        std::cerr << "No cluster centers to triangulate." << std::endl;
        return;
    } else if (clusterCenters.size() < 3) {
        std::cerr << "Not enough cluster centers to triangulate." << std::endl;
        return;
    }

    std::cout << "Creating Delaunay triangulation from "<< clusterCenters.size() << " cluster centers." << std::endl;

    // Store mapping from vertex to cluster center index
    std::unordered_map<Delaunay2D::DelaunayVertex, int, Delaunay2D::DelaunayVertexHash> vertexToClusterIndex;

    for (size_t i = 0; i < clusterCenters.size(); ++i) {
        const auto& center = clusterCenters[i];
        Delaunay2D::DelaunayVertex vertex(center.x, center.y);
        delaunayVertices.push_back(vertex);
        vertexToClusterIndex[vertex] = static_cast<int>(i); // Store the index of the cluster center
    }

    std::cout << "Running triangulation with " << delaunayVertices.size() << " vertices." << std::endl;

    // Perform Delaunay triangulation
    Delaunay2D delaunay = Delaunay2D::triangulate(delaunayVertices);

    std::cout << "Triangulation complete." << std::endl;

    // Convert edges to path connections
    selectedEdges.clear();

    if (delaunay.edges.empty()) {
        std::cerr << "No edges found in triangulation." << std::endl;
        return;
    }

    for (const auto& edge : delaunay.edges) {
        auto uIt = vertexToClusterIndex.find(edge.u);
        auto vIt = vertexToClusterIndex.find(edge.v);

        if (uIt == vertexToClusterIndex.end() || vIt == vertexToClusterIndex.end()) {
            std::cerr << "Could not find cluster index for edge vertex." << std::endl;
            continue;
        }

        int uId = uIt->second;
        int vId = vIt->second;

        if (uId < 0 || uId >= static_cast<int>(clusterCenters.size()) ||
            vId < 0 || vId >= static_cast<int>(clusterCenters.size())) {
            std::cerr << "Invalid cluster index: " << uId << ", " << vId << std::endl;
            continue;
        }

        std::cout << "Connecting cluster " << uId << " to cluster " << vId << std::endl;
        // If Ids are valid, add the edge to the selected edges
        selectedEdges.emplace_back(
            glm::ivec2(clusterCenters[uId]),
            glm::ivec2(clusterCenters[vId])
        );
        std::cout << "Edge added: " << uId << ", " << vId << std::endl;
    }

    std::cout << "Selected " << selectedEdges.size() << " edges for path generation." << std::endl;
}

void LibraryGen::addShelfWalls() {
    std::cout << "Adding walls around shelves..." << std::endl;

    // Add walls around the shelves - Direction for the 4-connected neighbors
    const std::vector<glm::ivec2> directions = {
        {1, 0}, {-1, 0}, {0, 1}, {0, -1}
    };

    // Temporary grid to track new shelves/walls
    Grid<CellType> newShelves(grid.getSize(), {0, 0}, NONE);

    // Add walls around the paths
    int wallsAdded = 0;
    for (int y = 0; y < grid.getSize().y; ++y) {
        for (int x = 0; x < grid.getSize().x; ++x) {
            glm::ivec2 pos(x, y);

            // Verify if the position is in bounds (should be, but just in case)
            if (!grid.inBounds(pos)) {
                continue;
            }

            if (grid[pos] == PATH) {
                // Check all 4 directions
                for (const auto& dir : directions) {
                    glm::ivec2 neighborPos = pos + dir;
                    if (grid.inBounds(neighborPos) && grid[neighborPos] == NONE) {
                        // If the neighbor is empty, mark it as a wall
                        newShelves[neighborPos] = SHELF;
                        wallsAdded++;
                    }
                }
            }
        }
    }

    // Merge new shelves into the main grid
    for (int y = 0; y < grid.getSize().y; ++y) {
        for (int x = 0; x < grid.getSize().x; ++x) {
            glm::ivec2 pos(x, y);

            // Verify if the position is in bounds (should be, but just in case)
            if (!newShelves.inBounds(pos)) {
                continue;
            }

            if (newShelves[pos] == SHELF) {
                grid[pos] = SHELF;
            }
        }
    }

    std::cout << "Added " << wallsAdded << " walls around shelves." << std::endl;

}


PathCost LibraryGen::calcCost(const glm::ivec2& from, const glm::ivec2& to) {
    PathCost cost;
    cost.traversable = grid.inBounds(to) && grid[to] != SHELF;

    if (grid[to] == PATH) {
        cost.cost = 1.0f; // Path cost
    } else if (grid[to] == NONE) {
        cost.cost = 5.0f; // Empty space cost
    } else {
        cost.cost = 0.0f; // Shelf cost (not traversable)
    }

    return cost;
}



