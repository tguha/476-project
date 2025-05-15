#include "AssimpMesh.h"
#include "Program.h"
#include "TextureManager.h"

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

    // Tangents
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));

    // Bitangents
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));

    glBindVertexArray(0);

    // std::cout << "Mesh setup complete" << std::endl;
}

// render the mesh
void AssimpMesh::Draw(const std::shared_ptr<Program> prog) const {
    // build an ID array in the same order as uMaps[0..5]
    GLuint ids[6] = { 0,0,0,0,0,0 };
    for (auto& t : textures) {
        if (t.type == "texture_diffuse")   ids[0] = t.id;
        if (t.type == "texture_specular")  ids[1] = t.id;
        if (t.type == "texture_roughness") ids[2] = t.id;
        if (t.type == "texture_metalness") ids[3] = t.id;
        if (t.type == "texture_normal")     ids[4] = t.id;
        if (t.type == "texture_emission")   ids[5] = t.id;
    }

    // bind each either to the real map or your 1×1 fallback
    for (int i = 0; i < 6; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        GLuint toBind = ids[i]
            ? ids[i]
            : (i == 4 ? TextureManager::flatNormal()
                : (i == 5 ? TextureManager::black() // <-- black for emission
                    : TextureManager::white())); // white for all others
        glBindTexture(GL_TEXTURE_2D, toBind);
    }

    // draw mesh
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // Reset to default texture unit
    glActiveTexture(GL_TEXTURE0);
}