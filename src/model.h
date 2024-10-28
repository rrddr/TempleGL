#ifndef TEMPLEGL_SRC_MODEL_H_
#define TEMPLEGL_SRC_MODEL_H_

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/glm.hpp>

#include <vector>
#include <string>
#include <memory>

struct DrawElementsIndirectCommand {
  unsigned int count;
  unsigned int instance_count;
  unsigned int first_vertex;
  int          base_vertex;
  unsigned int base_instance;
};

struct CustomStbiDeleter {
  void operator()(unsigned char* data);
};

class Model {
 public:
  struct Vertex {
    float position[3];
    float uv[2];
  };
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  std::vector<std::unique_ptr<unsigned char, CustomStbiDeleter>> texture_data;
  std::vector<DrawElementsIndirectCommand> draw_commands;

  explicit Model(const std::string& obj_path);

 private:
  std::string source_dir;

  void loadTextureData(aiMaterial** materials, unsigned int num_materials);
  void loadVertexData(aiMesh** meshes, unsigned int num_meshes);
};

#endif //TEMPLEGL_SRC_MODEL_H_
