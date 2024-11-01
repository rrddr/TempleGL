#include "model.h"

#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <assimp/Importer.hpp>

#include <format>
#include <iostream>


void CustomStbiDeleter::operator()(unsigned char* data) {
  stbi_image_free(data);
}

Model::Model(const std::string& obj_path) {
  // Importer keeps ownership of all assimp resources, and destroys them once it goes out of scope
  Assimp::Importer importer;
  const aiScene* scene = importer.ReadFile(obj_path, aiProcess_Triangulate);

  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
    glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_ERROR, 0, GL_DEBUG_SEVERITY_HIGH, -1,
                         std::format("Model load error: Failed to read file from path '{}'", obj_path).c_str());
    return;
  }

  source_dir = obj_path.substr(0, obj_path.find_last_of('/') + 1);
  loadTextureData(scene->mMaterials, scene->mNumMaterials);
  loadVertexData(scene->mMeshes, scene->mNumMeshes);

}

void Model::loadTextureData(aiMaterial** materials, unsigned int num_materials) {
  for (unsigned int i = 0; i < num_materials; ++i) {
    aiString material_name = materials[i]->GetName();
    for (auto folder : {"diffuse/", "specular/", "normal/"}) {
      std::string path = source_dir + folder + material_name.C_Str() + ".png";
      // Data loaded with stb_image must be deallocated with a special handle. Custom deleter takes care of this.
      int width, height, nrComponents;
      texture_data.emplace_back(stbi_load(path.c_str(), &width, &height, &nrComponents, STBI_rgb), CustomStbiDeleter());
      if (texture_data.back() == nullptr) {
        glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_ERROR, 0, GL_DEBUG_SEVERITY_MEDIUM, -1,
                             std::format("Texture load error: Failed to read file from path '{}'", path).c_str());
      }
    }
  }
}

void Model::loadVertexData(aiMesh** meshes, unsigned int num_meshes) {
  Vertex vertex {};
  int base_vertex = 0;
  unsigned int first_index = 0;
  for (unsigned int i = 0; i < num_meshes; ++i) {
    aiMesh* mesh = meshes[i];

    /// Instead of storing meta-information somewhere, we use it to directly create a draw command for the mesh
    draw_commands.emplace_back(mesh->mNumFaces*3, 1, first_index, base_vertex, mesh->mMaterialIndex);

    /// Append vertex data
    for (unsigned int j = 0; j < mesh->mNumVertices; ++j) {
      vertex.position[0] = mesh->mVertices[j].x;
      vertex.position[1] = mesh->mVertices[j].y;
      vertex.position[2] = mesh->mVertices[j].z;

      if (mesh->mTextureCoords[0]) {
        vertex.uv[0] = mesh->mTextureCoords[0][j].x;
        vertex.uv[1] = mesh->mTextureCoords[0][j].y;
      } else {
        vertex.uv[0] = 0.0;
        vertex.uv[1] = 0.0;
      }
      vertices.push_back(vertex);
      ++base_vertex;
    }

    /// Append index data
    for (unsigned int j = 0; j < mesh->mNumFaces; ++j) {
      aiFace face = mesh->mFaces[j];
      if (face.mNumIndices != 3) { continue; } // ignore this edge case, we should only ever have triangle faces
      indices.push_back(face.mIndices[0]);
      indices.push_back(face.mIndices[1]);
      indices.push_back(face.mIndices[2]);
      first_index += 3;
    }
  }
}