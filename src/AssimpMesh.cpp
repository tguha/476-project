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

    // Vertex tangent
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));

    // Vertex bitangent
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));

    // Bone IDs
    glEnableVertexAttribArray(5);
    // glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, m_BoneIDs));
    // glVertexAttribPointer(5, 4, GL_INT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_BoneIDs));
    glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, m_BoneIDs));

    // Weights
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_Weights));

    glBindVertexArray(0);

    // std::cout << "Mesh setup complete" << std::endl;
}

// each diffuse texture should be named as texture_diffuseN, where N is a number
// each specular texture should be named as texture_specularN, where N is a number
// uniform sampler2D texture_diffuseN
// uniform sampler2D texture_specularN

// by this naming convention we can define as many texture samplers as we want in the shaders and
// if a mesh actually does contain (so many) tetxures it will be loaded and applied

// render the mesh
void AssimpMesh::Draw(const std::shared_ptr<Program> prog) const {
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;
    unsigned int normalNr = 1;
    unsigned int heightNr = 1;
    unsigned int roughnessNr = 1;
    unsigned int metalnessNr = 1;
    unsigned int emissionNr = 1;

    for (unsigned int i = 0; i < textures.size(); i++) {
        glActiveTexture(GL_TEXTURE0 + i); // activate proper texture unit before binding
        // retrieve texture number (the N in diffuse_textureN)
        std::string number;
        std::string name = textures[i].type;
        if (name == "texture_diffuse") {
            number = std::to_string(diffuseNr++);
        } else if (name == "texture_specular") {
            number = std::to_string(specularNr++);
        } else if (name == "texture_normal") {
            number = std::to_string(normalNr++);
        } else if (name == "texture_height") {
            number = std::to_string(heightNr++);
        } else if (name == "texture_roughness") {
            number = std::to_string(roughnessNr++);
        } else if (name == "texture_metalness") {
            number = std::to_string(metalnessNr++);
        } else if (name == "texture_emission") {
            number = std::to_string(emissionNr++);
        }

        glUniform1i(glGetUniformLocation(prog->getPid(), (name + number).c_str()), i);
        glBindTexture(GL_TEXTURE_2D, textures[i].id);
    }

    // draw mesh
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // always good practice to set everything back to defaults once configured.
    glActiveTexture(GL_TEXTURE0);
}