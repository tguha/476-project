#ifndef BOOKSHELF_MANAGER_H
#define BOOKSHELF_MANAGER_H

#include <vector>
#include <glm/glm.hpp>
#include "LibraryGen.h"

class BookshelfManager {
    public:
        void generateShelves(const LibraryGen& generator);
        const std::vector<glm::mat4>& getShelfTransforms() const { return shelfTransforms; }
    private:
        std::vector<glm::mat4> shelfTransforms;
        const float SHELF_WIDTH = 0.5f;
        const float SHELF_HEIGHT = 0.2f;

        void addShelf(const glm::vec3& position, const glm::vec3& rotation);
};

#endif