#ifndef GRID_H
#define GRID_H

#include <vector>
#include <glm/glm.hpp>
#include <iostream>

template<typename T>
class Grid {
    public:
        Grid(const glm::ivec2 size = glm::ivec2(1, 1), T defaultValue = T())
            : size(size){
                data.resize(size.x * size.y, defaultValue); // Initialize the grid with default values
                if (size.x <= 0 || size.y <= 0) {
                    throw std::invalid_argument("Grid size must be positive!");
                }
            }

        // Copy/move operations
        Grid(const Grid<T>& other) = default; // Copy constructor
        Grid& operator=(const Grid&) = default; // Copy assignment operator
        Grid(Grid&&) = default; // Move constructor
        Grid& operator=(Grid&&) = default; // Move assignment operator

        // Bounds-checked access operator
        T& operator[](const glm::ivec2& pos) {
            int x = pos.x;
            int y = pos.y;
            return data[y * size.x + x];
        }

        const T& operator[](const glm::ivec2& pos) const {
            int x = pos.x;
            int y = pos.y;
            return data[y * size.x + x];
        }

        // at() method for bounds-checked access
        T& at(int x, int y) {
            return data[y * size.x + x];
        }

        const T& at(int x, int y) const {
            return data[y * size.x + x];
        }

        bool inBounds(const glm::ivec2& pos) const {
            int x = pos.x;
            int y = pos.y;
            return x >= 0 && x < size.x && y >= 0 && y < size.y;
        }

        void printGrid() const {
            for (int y = 0; y < size.y; ++y) {
                for (int x = 0; x < size.x; ++x) {
                    std::cout << data[y * size.x + x] << " ";
                }
                std::cout << std::endl;
            }
        }

        // Getters for size and offset and cell access
        T& getCell(const glm::ivec2& pos) {
            return (*this)[pos];
        }

        const T& getCell(const glm::ivec2& pos) const {
            return (*this)[pos];
        }

        glm::ivec2 getSize() const { return size; }

        int mapXtoGridX(float x) const {
            // float worldXwidth = size.x;
            // return (x-(-(size.x))/worldXwidth) * size.x;
            float worldXWidth = size.x * 2;
            // return (x - (-size.x)) /  (worldXWidth / (size.x - 1));
            return static_cast<int>((x - (-size.x)) / (worldXWidth / (size.x - 1)));
        }

        int mapZtoGridY(float z) const {
            // float worldZwidth = size.y;
            // return (z-(-(size.y))/worldZwidth) * size.y;
            float worldZwidth = size.y * 2;
            // return (z - (-size.y)) / (worldZwidth / (size.y - 1));
            return static_cast<int>((z - (-size.y)) / (worldZwidth / (size.y - 1)));
        }

        float mapGridXtoWorldX(int x) const {
            float worldXwidth = size.x * 2;
            // return (x * (worldXwidth / (size.x - 1))) - size.x;
            return (-size.x) + (x * (worldXwidth / (size.x - 1)));
        }

        float mapGridYtoWorldZ(int y) const {
            float worldZwidth = size.y * 2;
            // return (y * (worldZwidth / (size.y - 1))) - size.y;
            return (-size.y) + (y * (worldZwidth / (size.y - 1)));
        }

    private:
        glm::ivec2 size;
        glm::ivec2 offset;
        std::vector<T> data;

};

#endif // GRID_H