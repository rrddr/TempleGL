#include "model.h"
#include "stbi_helpers.h"

#include <glad/glad.h>
#include <assimp/postprocess.h>

#include <format>
#include <filesystem>
#include <cstring>

Model::Model(std::string folder_path)
  : source_dir_ {std::move(folder_path)} {
  loadModelData();
  loadLightData();
}

void Model::drawSetup(const GLuint vertex_buffer_binding, const GLuint texture_binding) const {
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, vertex_buffer_binding, vertex_buffer_.id);
  glBindTextureUnit(texture_binding, texture_array_.id);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_.id);
  glBindBuffer(GL_DRAW_INDIRECT_BUFFER, draw_command_buffer_.id);
}

void Model::loadModelData() {
  const std::string path {source_dir_ + "model.obj"};
  const aiScene* scene {importer_.ReadFile(path.c_str(),
                                           aiProcess_FlipUVs |
                                           aiProcess_Triangulate |
                                           aiProcess_GenNormals |
                                           aiProcess_CalcTangentSpace)};
  checkAssimpSceneErrors(scene, path);
  createTextureArray(scene->mMaterials, scene->mNumMaterials);
  createBuffers(scene->mMeshes, scene->mNumMeshes);
  glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION,
                       GL_DEBUG_TYPE_OTHER,
                       0,
                       GL_DEBUG_SEVERITY_NOTIFICATION,
                       -1,
                       "(Model::loadModelData): Completed successfully.");
}

void Model::loadLightData() {
  const std::string path {source_dir_ + "lights.obj"};
  if (!std::filesystem::exists(path)) return;
  const aiScene* scene {importer_.ReadFile(path.c_str(), 0)};
  checkAssimpSceneErrors(scene, path);
  for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
    const aiMesh* mesh {scene->mMeshes[i]};
    if (std::strcmp(scene->mMaterials[mesh->mMaterialIndex]->GetName().C_Str(), "light_source") != 0) continue;
    for (unsigned int j = 0; j < mesh->mNumFaces; ++j) {
      const aiFace& face {mesh->mFaces[j]};
      aiVector3t average {0.0f};
      for (unsigned int k = 0; k < face.mNumIndices; ++k) {
        average += mesh->mVertices[face.mIndices[k]];
      }
      average /= static_cast<float>(face.mNumIndices);
      light_positions_.emplace_back(average.x, average.y, average.z, 1.0f);
    }
  }
  glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION,
                       GL_DEBUG_TYPE_OTHER,
                       0,
                       GL_DEBUG_SEVERITY_NOTIFICATION,
                       -1,
                       "(Model::loadLightData): Completed successfully.");
}

void Model::createTextureArray(aiMaterial** materials, const unsigned int num_materials) {
  glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &texture_array_.id);
  glTextureStorage3D(texture_array_.id, 1, GL_RGB8, TEX_SIZE, TEX_SIZE, static_cast<GLsizei>(num_materials) * 3);
  GLint layer {0};
  for (unsigned int i = 0; i < num_materials; ++i) {
    auto material_name {materials[i]->GetName().C_Str()};
    for (auto folder : {"diffuse/", "normal/", "specular/"}) {
      std::string path {source_dir_ + folder + material_name + ".png"};
      if (!std::filesystem::exists(path)) {
        glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION,
                             GL_DEBUG_TYPE_OTHER,
                             texture_array_.id,
                             GL_DEBUG_SEVERITY_NOTIFICATION,
                             -1,
                             std::format("(Model::createTextureArray): Using default {} texture for material '{}.'",
                                         folder,
                                         material_name).c_str());
        path = source_dir_ + folder + "DefaultMaterial.png";
      }
      help::fill3DTextureLayer(path, texture_array_, layer, TEX_SIZE, TEX_SIZE);
      ++layer;
    }
  }
  glTextureParameteri(texture_array_.id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTextureParameteri(texture_array_.id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION,
                       GL_DEBUG_TYPE_OTHER,
                       texture_array_.id,
                       GL_DEBUG_SEVERITY_NOTIFICATION,
                       -1,
                       "(Model::createTextureArray): Completed successfully.");
}

void Model::createBuffers(aiMesh** meshes, const unsigned int num_meshes) {
  std::vector<Vertex> vertices;
  std::vector<GLuint> indices;
  std::vector<DrawElementsIndirectCommand> draw_commands;
  draw_commands.reserve(num_meshes);

  GLint base_vertex {0};
  GLuint first_index {0};
  for (unsigned int i = 0; i < num_meshes; ++i) {
    const aiMesh* mesh {meshes[i]};
    if (mesh->mPrimitiveTypes != (aiPrimitiveType_TRIANGLE | aiPrimitiveType_NGONEncodingFlag)) {
      glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION,
                           GL_DEBUG_TYPE_ERROR,
                           0,
                           GL_DEBUG_SEVERITY_MEDIUM,
                           -1,
                           "(Model::createBuffers): Detected point/line primitives in mesh, which are not allowed. "
                           "This mesh will be skipped.");
      continue;
    }
    draw_commands.emplace_back(mesh->mNumFaces * 3, 1, first_index, base_vertex, mesh->mMaterialIndex);
    for (unsigned int j = 0; j < mesh->mNumVertices; ++j) {
      vertices.emplace_back(Vertex {{mesh->mVertices[j].x,
                                     mesh->mVertices[j].y,
                                     mesh->mVertices[j].z},
                                    {mesh->mTangents[j].x,
                                     mesh->mTangents[j].y,
                                     mesh->mTangents[j].z},
                                    {mesh->mBitangents[j].x,
                                     mesh->mBitangents[j].y,
                                     mesh->mBitangents[j].z},
                                    {mesh->mTextureCoords[0] ? mesh->mTextureCoords[0][j].x : 0.0f,
                                     mesh->mTextureCoords[0] ? mesh->mTextureCoords[0][j].y : 0.0f}});
      ++base_vertex;
    }
    for (unsigned int j = 0; j < mesh->mNumFaces; ++j) {
      const aiFace& face {mesh->mFaces[j]};
      indices.emplace_back(face.mIndices[0]);
      indices.emplace_back(face.mIndices[1]);
      indices.emplace_back(face.mIndices[2]);
      first_index += 3;
    }
  }
  num_draw_commands_ = static_cast<GLsizei>(std::ssize(draw_commands));
  createBufferFromVector<Vertex>(vertex_buffer_, vertices);
  createBufferFromVector<GLuint>(index_buffer_, indices);
  createBufferFromVector<DrawElementsIndirectCommand>(draw_command_buffer_, draw_commands);
  glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION,
                       GL_DEBUG_TYPE_OTHER,
                       0,
                       GL_DEBUG_SEVERITY_NOTIFICATION,
                       -1,
                       "(Model::createBuffers): Completed successfully.");
}

template <typename T>
void Model::createBufferFromVector(wrap::Buffer& buffer, const std::vector<T>& vector) {
  glCreateBuffers(1, &buffer.id);
  glNamedBufferStorage(buffer.id, sizeof(T) * std::ssize(vector), vector.data(), GL_DYNAMIC_STORAGE_BIT);
}

void Model::checkAssimpSceneErrors(const aiScene* scene, const std::string& path) const {
  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
    glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION,
                         GL_DEBUG_TYPE_ERROR,
                         0,
                         GL_DEBUG_SEVERITY_HIGH,
                         -1,
                         std::format("(Model::checkAssimpSceneErrors): Failed to read file from path '{}'. "
                                     "Reason: '{}'",
                                     path,
                                     importer_.GetErrorString()).c_str());
  }
}
