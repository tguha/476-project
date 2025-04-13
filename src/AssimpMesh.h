#ifndef ASSIMPMESH_H
#define ASSIMPMESH_H

#include <string>
#include <vector>
#include <memory>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

#include "Program.h"

#define MAX_BONE_INFLUENCE 4


struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Tangent;
    glm::vec3 Bitangent;

    int m_BoneIDs[MAX_BONE_INFLUENCE];
    float m_Weights[MAX_BONE_INFLUENCE];
};

struct AssimpTexture {
    unsigned int id;
    std::string type;
    std::string path;
};

class AssimpMesh {
    public:
       std::vector<Vertex> vertices;
       std::vector<unsigned int> indices;
       std::vector<AssimpTexture> textures;
       unsigned int VAO;

       AssimpMesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<AssimpTexture> textures);
       void Draw(const std::shared_ptr<Program> prog) const;

    private:
        unsigned int VBO, EBO;

        void setupMesh();
};

#endif // ASSIMPMESH_H