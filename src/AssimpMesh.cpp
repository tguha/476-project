#include "AssimpMesh.h"
#include "Program.h"

#include <iostream>

// Constructor
AssimpMesh::AssimpMesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<AssimpTexture> textures) {
    this->vertices = vertices;
    this->indices = indices;
    this->textures = textures;

    // std::cout << "Mesh created" << std::endl;

    setupMesh();
}

void AssimpMesh::setupMesh()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // Vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

    // Vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

    // Vertex texture coordinates
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

    // Bone IDs
    glEnableVertexAttribArray(3);
    glVertexAttribIPointer(3, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, m_BoneIDs));

    // Weights
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_Weights));

    glBindVertexArray(0);

    // std::cout << "Mesh setup complete" << std::endl;
}

// render the mesh
void AssimpMesh::Draw(const std::shared_ptr<Program> prog) const {
    // Map to keep track of texture types we've already bound
    std::map<std::string, bool> boundTextures;

    // Simple mapping from type to uniform name
    std::map<std::string, std::string> typeToUniform = {
        {"texture_diffuse", "TexDif"},
        {"texture_specular", "TexSpec"},
        {"texture_normal", "TexNor"},
        {"texture_roughness", "TexRough"},
        {"texture_metalness", "TexMet"},
        {"texture_emission", "TexEmit"}
    };

    // Corresponding boolean uniform names
    std::map<std::string, std::string> typeToBoolUniform = {
        {"texture_diffuse", "hasTexDif"},
        {"texture_specular", "hasTexSpec"},
        {"texture_normal", "hasTexNor"},
        {"texture_roughness", "hasTexRough"},
        {"texture_metalness", "hasTexMet"},
        {"texture_emission", "hasTexEmit"}
    };

    // First, set all texture availability flags to false
    for (const auto& pair : typeToBoolUniform) {
        if (prog->getUniform(pair.second) != -1) {
            glUniform1i(prog->getUniform(pair.second), 0);
        }
    }

    int textureUnit = 0;
    for (const auto& texture : textures) {
        std::string uniformName = typeToUniform[texture.type];
        std::string boolUniformName = typeToBoolUniform[texture.type];

        // Only bind the first texture of each type and it exists
        if (prog->getUniform(uniformName) != -1 && boundTextures.find(texture.type) == boundTextures.end()) {
            glActiveTexture(GL_TEXTURE0 + textureUnit);
            glBindTexture(GL_TEXTURE_2D, texture.id);
            glUniform1i(prog->getUniform(uniformName), textureUnit);

            // Set the boolean flag to indicate this texture is available
            if (prog->getUniform(boolUniformName) != -1) {
                glUniform1i(prog->getUniform(boolUniformName), 1);
            }

            boundTextures[texture.type] = true;
            textureUnit++;
        }
    }

    // draw mesh
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // Reset to default texture unit
    glActiveTexture(GL_TEXTURE0);
}