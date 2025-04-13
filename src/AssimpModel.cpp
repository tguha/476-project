#include "AssimpModel.h"
#include "Program.h"
#include <iostream>
#include "stb_image.h"
#include "AssimpGLMHelpers.h"


AssimpModel::AssimpModel(std::string const &path, bool gamma) : gammaCorrection(gamma) {
    loadModel(path);
    // std::cout << "Model: " << path << " loaded" << std::endl;
}

AssimpModel::~AssimpModel() {
}

void AssimpModel::Draw(const std::shared_ptr<Program> prog) const {
    // std::cout << "Mesh size: " << meshes.size() << std::endl;
    for (unsigned int i = 0; i < meshes.size(); i++) {
        meshes[i].Draw(prog);
        // std::cout << "Drawing mesh: " << i << std::endl;
    }
}

void AssimpModel::loadModel(std::string const &path) {
    Assimp::Importer importer;
    // importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
    // importer.SetPropertyFloat(AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, 1.0f);
    const aiScene *scene = importer.ReadFile(path,
        aiProcess_Triangulate |
        aiProcess_GenSmoothNormals |
        aiProcess_CalcTangentSpace |
        aiProcess_ValidateDataStructure |
        aiProcess_PopulateArmatureData |
        // aiProcess_LimitBoneWeights |
        // aiProcess_GlobalScale |
        aiProcess_GenBoundingBoxes);

    std::cout << "Loading model: " << path << std::endl;
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return;
    }

    for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
        aiMesh* mesh = scene->mMeshes[i];
        const aiAABB& aabb = mesh->mAABB;

        // std::cout << "Mesh " << i << " AABB min: (" << aabb.mMin.x << ", " << aabb.mMin.y << ", " << aabb.mMin.z << ")" << std::endl;
        // std::cout << "Mesh " << i << " AABB max: (" << aabb.mMax.x << ", " << aabb.mMax.y << ", " << aabb.mMax.z << ")" << std::endl;

        m_BoundingBoxes.push_back(aabb);
    }

    // calculate the bounding box for the model
    for (const auto& box : m_BoundingBoxes) {
        boundingBoxMin.x = std::min(boundingBoxMin.x, box.mMin.x);
        boundingBoxMin.y = std::min(boundingBoxMin.y, box.mMin.y);
        boundingBoxMin.z = std::min(boundingBoxMin.z, box.mMin.z);

        boundingBoxMax.x = std::max(boundingBoxMax.x, box.mMax.x);
        boundingBoxMax.y = std::max(boundingBoxMax.y, box.mMax.y);
        boundingBoxMax.z = std::max(boundingBoxMax.z, box.mMax.z);
    }

    // std::cout << "Model: " << path << " bounding box min: (" << boundingBoxMin.x << ", " << boundingBoxMin.y << ", " << boundingBoxMin.z << ")" << std::endl;
    // std::cout << "Model: " << path << " bounding box max: (" << boundingBoxMax.x << ", " << boundingBoxMax.y << ", " << boundingBoxMax.z << ")" << std::endl;


    std::cout<<"Loaded model: "<<path<<std::endl;

    directory = path.substr(0, path.find_last_of('/'));

    // boundingBoxMin = glm::vec3(std::numeric_limits<float>::max());
    // boundingBoxMax = glm::vec3(std::numeric_limits<float>::lowest());

    processNode(scene->mRootNode, scene);

    // after processing all nodes, we can calculate the bounding box
    // calculateBoundingBox();

    std::cout<<"Model loaded"<<std::endl;
}

void AssimpModel::processNode(aiNode* node, const aiScene* scene) {

    // Process all the node's meshes (if any)
    // std::cout<< "Processing node: " << node->mName.C_Str() << std::endl;
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        // std::cout<<"Processing node: "<<i<<std::endl;
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }

    // std::cout<<"Node processed"<<std::endl;

    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        // std::cout<<"Processing children: "<<i<<std::endl;
        processNode(node->mChildren[i], scene);
    }

    // std::cout<<"Children processed"<<std::endl;
}

std::vector<AssimpTexture> AssimpModel::loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName)
{
    std::vector<AssimpTexture> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
        aiString str;
        mat->GetTexture(type, i, &str);
        bool skip = false;
        for (unsigned int j = 0; j < textures_loaded.size(); j++) {
            if (std::strcmp(textures_loaded[j].path.c_str(), str.C_Str()) == 0) {
                textures.push_back(textures_loaded[j]);
                skip = true;
                break;
            }
        }
        if (!skip) {
            AssimpTexture texture;
            texture.id = AssimpTextureFromFile(str.C_Str(), directory);
            texture.type = typeName;
            texture.path = str.C_Str();
            textures.push_back(texture);
            textures_loaded.push_back(texture);
        }
    }

    return textures;
}

unsigned int AssimpTextureFromFile(const char *path, const std::string &directory, bool gamma) {
    std::string filename = std::string(path);
    filename = directory + '/' + filename;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format;
        if (nrComponents == 1) {
            format = GL_RED;
        } else if (nrComponents == 3) {
            format = GL_RGB;
        } else if (nrComponents == 4) {
            format = GL_RGBA;
        }

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    } else {
        std::cerr << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

void AssimpModel::SetVertexBoneDataToDefault(Vertex& vertex) {
    for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
        vertex.m_BoneIDs[i] = -1;
        vertex.m_Weights[i] = 0.0f;
    }
}

AssimpMesh AssimpModel::processMesh(aiMesh* mesh, const aiScene* scene) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<AssimpTexture> textures;

    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;
        SetVertexBoneDataToDefault(vertex);
        vertex.Position = AssimpGLMHelpers::GetGLMVec(mesh->mVertices[i]);
        vertex.Normal = AssimpGLMHelpers::GetGLMVec(mesh->mNormals[i]);

        if (mesh->mTextureCoords[0])
        {
            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.TexCoords = vec;
        }
        else {
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);
        }

        // std::cout << "Processing vertex: " << i << std::endl;

        vertices.push_back(vertex);

        // update bounding box based on min/max for the model
        // boundingBoxMin = glm::min(boundingBoxMin, vertex.Position);
        // boundingBoxMax = glm::max(boundingBoxMax, vertex.Position);
    }
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }

    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

          // diffuse maps
    std::vector<AssimpTexture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

    // specular maps
    std::vector<AssimpTexture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

    // normal maps
    std::vector<AssimpTexture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
    textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

    // height maps
    std::vector<AssimpTexture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
    textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

    // roughness maps
    std::vector<AssimpTexture> roughnessMaps = loadMaterialTextures(material, aiTextureType_SHININESS, "texture_roughness");
    textures.insert(textures.end(), roughnessMaps.begin(), roughnessMaps.end());

    // metalness maps
    std::vector<AssimpTexture> metalnessMaps = loadMaterialTextures(material, aiTextureType_OPACITY, "texture_metalness");
    textures.insert(textures.end(), metalnessMaps.begin(), metalnessMaps.end());

    // emission maps
    std::vector<AssimpTexture> emissionMaps = loadMaterialTextures(material, aiTextureType_EMISSIVE, "texture_emission");
    textures.insert(textures.end(), emissionMaps.begin(), emissionMaps.end());

    ExtractBoneWeightForVertices(vertices, mesh, scene);

    // std::cout << "Mesh processed" << std::endl;

    return AssimpMesh(vertices, indices, textures);
}

void AssimpModel::SetVertexBoneData(Vertex& vertex, int boneID, float weight) {
    // check for zero weights
    if (weight <= 0.0f) {
        return;
    }

    // check for duplicate bone IDs
    for (int i = 0; i < MAX_BONE_INFLUENCE; ++i) {
        if (vertex.m_BoneIDs[i] == boneID) {
            return; // already set
        }
    }


    for (int i = 0; i < MAX_BONE_INFLUENCE; ++i) {
        if (vertex.m_BoneIDs[i] < 0) {
            vertex.m_Weights[i] = weight;
            vertex.m_BoneIDs[i] = boneID;

            // std::cout << "Bone ID: " << boneID << " Weight: " << weight << std::endl;
            break;
        }
    }
}

void AssimpModel::ExtractBoneWeightForVertices(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene) {
    auto& boneInfoMap = m_BoneInfoMap;
    auto& boneCounter = m_BoneCounter;

    // std::cout << "Bone counter: " << boneCounter << std::endl;

    for (int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
        int boneID = -1;
        std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
        if (boneInfoMap.find(boneName) == boneInfoMap.end()) {
            BoneInfo newBoneInfo;
            newBoneInfo.id = boneCounter;
            newBoneInfo.offset = AssimpGLMHelpers::ConvertMatrixToGLMFormat(mesh->mBones[boneIndex]->mOffsetMatrix);
            m_BoneInfoMap[boneName] = newBoneInfo;
            boneID = boneCounter;
            boneCounter++;
        } else {
            boneID = m_BoneInfoMap[boneName].id;
        }

        assert(boneID != -1);
        auto weights = mesh->mBones[boneIndex]->mWeights;
        int numWeights = mesh->mBones[boneIndex]->mNumWeights;

        for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex) {
            int vertexID = weights[weightIndex].mVertexId;
            float weight = weights[weightIndex].mWeight;
            assert(vertexID <= vertices.size());
            SetVertexBoneData(vertices[vertexID], boneID, weight);
        }
    }

    // normalize weights
    for (auto& vertex : vertices) {
        float totalWeight = 0.0f;
        for (int i = 0; i < MAX_BONE_INFLUENCE; ++i) {
            totalWeight += vertex.m_Weights[i];
        }
        if (totalWeight > 0.0f) {
            for (int i = 0; i < MAX_BONE_INFLUENCE; ++i) {
                vertex.m_Weights[i] /= totalWeight;
            }
        }
    }

}

void AssimpModel::calculateBoundingBox() {
    // Iterate over all meshes and their vertices to find the min and max coordinates
    for (const auto& mesh : meshes) {
        for (const auto& vertex : mesh.vertices) {
            boundingBoxMin = glm::min(boundingBoxMin, vertex.Position);
            boundingBoxMax = glm::max(boundingBoxMax, vertex.Position);
        }
    }
}
