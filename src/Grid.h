#ifndef GRID_H
#define GRID_H

#include <vector>
#include <glm/glm.hpp>
#include <iostream>

template<typename T>
class Grid {
    public:
        Grid(glm::ivec2 size = {1, 1}, glm::ivec2 offset = {0, 0}, T defaultValue = T())
            : size(size), offset(offset), data(size.x * size.y, defaultValue) {
                if (size.x <= 0 || size.y <= 0) {
                    throw std::invalid_argument("Grid size must be positive!");
                }
            }

        // Copy/move operations
        Grid(const Grid<T>& other) = default; // Copy constructor
        Grid& operator=(const Grid&) = default; // Copy assignment operator
        Grid(Grid&&) = default; // Move constructor
        Grid& operator=(Grid&&) = default; // Move assignment operator

        // Copy assignment operator
        // Grid<T>& operator=(const Grid<T>& other) {
        //     if (this != &other) {
        //         size = other.size;
        //         offset = other.offset;
        //         data = other.data;
        //     }
        //     return *this;
        // }

        // Bounds-checked access operator
        T& operator[](const glm::ivec2& pos) {
            glm::ivec2 adjusted = pos + offset;
            return data[adjusted.y * size.x + adjusted.x];
        }

        const T& operator[](const glm::ivec2& pos) const {
            glm::ivec2 adjusted = pos + offset;
            return data[adjusted.y * size.x + adjusted.x];
        }

        bool inBounds(const glm::ivec2& pos) const {
            glm::ivec2 adjusted = pos + offset;
            return adjusted.x >= 0 && adjusted.x < size.x &&
                   adjusted.y >= 0 && adjusted.y < size.y;
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


        glm::ivec2 getOffset() const { return offset; }
        glm::ivec2 getSize() const { return size; }

    private:
        glm::ivec2 size;
        glm::ivec2 offset;
        std::vector<T> data;

};

#endif // GRID_H