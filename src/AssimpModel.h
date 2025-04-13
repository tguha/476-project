#ifndef ASSIMPMODEL_H
#define ASSIMPMODEL_H

#include <string>
#include <vector>
#include <memory>
#include <limits>
#include <algorithm>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "AssimpMesh.h" // Include AssimpMesh.h to use AssimpMesh class

using namespace glm;

unsigned int AssimpTextureFromFile(const char *path, const std::string &directory, bool gamma = false);

struct BoneInfo {
    // id is index in finalBoneMatrices
    int id;

    // offset matrix transforms vertex from model space to bone space
    mat4 offset;
};

class AssimpModel {
    public:
        AssimpModel(std::string const &path, bool gamma = false);
        ~AssimpModel();

        void Draw(const std::shared_ptr<Program> prog) const;


        auto& GetBoneInfoMap() { return m_BoneInfoMap; }
        int& GetBoneCounter() { return m_BoneCounter; }

        std::vector<AssimpMesh> meshes;
        std::string directory;
        std::vector<AssimpTexture> textures_loaded;
        bool gammaCorrection;

        glm::vec3 boundingBoxMin = glm::vec3(std::numeric_limits<float>::infinity());
        glm::vec3 boundingBoxMax;


    private:
        void loadModel(std::string const &path);
        void processNode(aiNode *node, const aiScene *scene);
        AssimpMesh processMesh(aiMesh *mesh, const aiScene *scene);
        std::vector<AssimpTexture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName);

    private:

        std::map<std::string, BoneInfo> m_BoneInfoMap;
        int m_BoneCounter = 0;

        void SetVertexBoneDataToDefault(Vertex& vertex);
        void SetVertexBoneData(Vertex& vertex, int boneID, float weight);
        void ExtractBoneWeightForVertices(std::vector<Vertex>& vertices, aiMesh *mesh, const aiScene *scene);
        void calculateBoundingBox();

        std::vector<aiAABB> m_BoundingBoxes;
};

#endif // ASSIMPMODEL_H