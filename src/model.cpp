#include "model.h"
#include "stbi_helpers.h"

#include <glad/glad.h>
#include <assimp/Importer.hpp>

#include <format>
#include <filesystem>
#include <cstring>

Model::Model(const std::string& obj_path) {
  // Importer keeps ownership of all assimp resources, and destroys them once it goes out of scope
  Assimp::Importer importer;
  const aiScene* scene {importer.ReadFile(obj_path,
                                          aiProcess_FlipUVs |
                                          aiProcess_Triangulate |
                                          aiProcess_GenNormals |
                                          aiProcess_CalcTangentSpace)};

  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
    glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_ERROR, 0, GL_DEBUG_SEVERITY_HIGH, -1,
                         std::format("(Model::Model): Failed to read file from path '{}'", obj_path).c_str());
    return;
  }
  source_dir_ = obj_path.substr(0, obj_path.find_last_of('/') + 1);
  num_draw_commands_ = static_cast<GLsizei>(scene->mNumMeshes);
  processMaterials(scene->mMaterials, scene->mNumMaterials);
  processMeshes(scene->mMeshes, scene->mNumMeshes);
}

void Model::drawSetup(GLuint vertex_buffer_binding,
                      GLuint texture_binding) const {
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, vertex_buffer_binding, vertex_buffer_.id);
  glBindTextureUnit(texture_binding, texture_array_.id);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_.id);
  glBindBuffer(GL_DRAW_INDIRECT_BUFFER, draw_command_buffer_.id);
}

void Model::processMaterials(aiMaterial** materials, unsigned int num_materials) {
  glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &texture_array_.id);
  glTextureStorage3D(texture_array_.id, 1, GL_RGB8, TEX_SIZE, TEX_SIZE, static_cast<GLsizei>(num_materials) * 3);
  GLint z_offset {0};
  for (unsigned int i = 0; i < num_materials; ++i) {
    auto material_name = materials[i]->GetName().C_Str();
    if (std::find_if(LIGHT_MATERIAL_NAMES.begin(),
                     LIGHT_MATERIAL_NAMES.end(),
                     [&material_name](const char* s) { return strcmp(s, material_name) == 0; })
        != LIGHT_MATERIAL_NAMES.end()) {
      light_material_indices_.push_back(i);
    }
    for (auto folder : {"diffuse/", "normal/", "specular/"}) {
      std::string path = source_dir_ + folder + material_name + ".png";
      if (!std::filesystem::exists(path)) {
        glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_OTHER, 0, GL_DEBUG_SEVERITY_NOTIFICATION, -1,
                             std::format("(Model::processMaterials): Using default {} texture for material '{}.'",
                                         folder, material_name).c_str());
        path = source_dir_ + folder + "DefaultMaterial.png";
      }
      help::fill3DTextureLayer(path, texture_array_, z_offset, TEX_SIZE, TEX_SIZE);
      ++z_offset;
    }
  }
  glTextureParameteri(texture_array_.id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTextureParameteri(texture_array_.id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_OTHER, 0, GL_DEBUG_SEVERITY_NOTIFICATION, -1,
                       "Model::processMaterials() successful.");
}

void Model::processMeshes(aiMesh** meshes, unsigned int num_meshes) {
  std::vector<Vertex> vertices;
  std::vector<GLuint> indices;
  std::vector<DrawElementsIndirectCommand> draw_commands;

  GLint base_vertex = 0;
  GLuint first_index = 0;
  Vertex vertex {};
  for (int i = 0; i < num_meshes; ++i) {
    aiMesh* mesh = meshes[i];
    if (mesh->mPrimitiveTypes != (aiPrimitiveType_TRIANGLE | aiPrimitiveType_NGONEncodingFlag)) {
      glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_ERROR, 0, GL_DEBUG_SEVERITY_MEDIUM, -1,
                           "(Model::processMeshes): Detected point/line primitives in mesh, which is not allowed. "
                           "This mesh will be skipped.");
      continue;
    }

    /// Check if mesh is a light source, and if so, store average of vertices as light position
    if (std::find(light_material_indices_.begin(), light_material_indices_.end(), mesh->mMaterialIndex)
        != light_material_indices_.end()) {
      aiVector3t<ai_real> average {0.0f};
      for (int j = 0; j < mesh->mNumVertices; ++j) {
//        average += mesh->mVertices[j];
          light_positions_.emplace_back(mesh->mVertices[j].x, mesh->mVertices[j].y, mesh->mVertices[j].z, 1.0f);
      }
//      average /= static_cast<float>(mesh->mNumVertices);
//      light_positions_.emplace_back(average.x, average.y, average.z, 1.0f);
    }

    /// Instead of storing meta-information somewhere, we use it to directly create a draw command for the mesh
    draw_commands.emplace_back(mesh->mNumFaces * 3, 1, first_index, base_vertex, mesh->mMaterialIndex);

    /// Append vertex data
    for (int j = 0; j < mesh->mNumVertices; ++j) {
      vertex.position[0] = mesh->mVertices[j].x;
      vertex.position[1] = mesh->mVertices[j].y;
      vertex.position[2] = mesh->mVertices[j].z;

      vertex.tangent[0] = mesh->mTangents[j].x;
      vertex.tangent[1] = mesh->mTangents[j].y;
      vertex.tangent[2] = mesh->mTangents[j].z;

      vertex.bitangent[0] = mesh->mBitangents[j].x;
      vertex.bitangent[1] = mesh->mBitangents[j].y;
      vertex.bitangent[2] = mesh->mBitangents[j].z;

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
    for (int j = 0; j < mesh->mNumFaces; ++j) {
      aiFace face = mesh->mFaces[j];
      indices.push_back(face.mIndices[0]);
      indices.push_back(face.mIndices[1]);
      indices.push_back(face.mIndices[2]);
      first_index += 3;
    }
  }

  /// Create buffers
  createBufferFromVector<Vertex>(vertex_buffer_, vertices);
  createBufferFromVector<GLuint>(index_buffer_, indices);
  createBufferFromVector<DrawElementsIndirectCommand>(draw_command_buffer_, draw_commands);

  glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_OTHER, 0, GL_DEBUG_SEVERITY_NOTIFICATION, -1,
                       "Model::processMeshes() successful.");
}

template<typename T>
void Model::createBufferFromVector(wrap::Buffer& buffer, const std::vector<T>& vector) {
  glCreateBuffers(1, &buffer.id);
  glNamedBufferStorage(buffer.id, sizeof(T) * static_cast<GLsizeiptr>(vector.size()),
                       vector.data(), GL_DYNAMIC_STORAGE_BIT);
}