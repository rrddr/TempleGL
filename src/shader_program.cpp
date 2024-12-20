#include "shader_program.h"

#include <fstream>
#include <format>
#include <array>

ShaderProgram::Stages::Stages(std::string source_directory,
                              const std::vector<std::pair<std::string, int>>& shader_constants)
  : source_dir_(std::move(source_directory)) {
  for (const auto& [name, value] : shader_constants) {
    shader_constants_.append(std::format("#define {} {}\n", name, value));
  }
}

ShaderProgram::Stages& ShaderProgram::Stages::vertex(const std::string& filename) {
  vertex_shader_source_ = loadShaderSource(filename);
  return *this;
}
ShaderProgram::Stages& ShaderProgram::Stages::tessellationControl(const std::string& filename) {
  tessellation_control_shader_source_ = loadShaderSource(filename);
  return *this;
}
ShaderProgram::Stages& ShaderProgram::Stages::tessellationEvaluation(const std::string& filename) {
  tessellation_evaluation_shader_source_ = loadShaderSource(filename);
  return *this;
}
ShaderProgram::Stages& ShaderProgram::Stages::geometry(const std::string& filename) {
  geometry_shader_source_ = loadShaderSource(filename);
  return *this;
}
ShaderProgram::Stages& ShaderProgram::Stages::fragment(const std::string& filename) {
  fragment_shader_source_ = loadShaderSource(filename);
  return *this;
}
ShaderProgram::Stages& ShaderProgram::Stages::compute(const std::string& filename) {
  compute_shader_source_ = loadShaderSource(filename);
  return *this;
}

std::string ShaderProgram::Stages::loadShaderSource(const std::string& filename) {
  std::ifstream shader_file;
  shader_file.exceptions(std::ifstream::badbit);
  try {
    std::string shader_source;
    shader_file.open(source_dir_ + filename);
    for (std::string line; std::getline(shader_file, line);) {
      if (line.compare(0, 8, "#version") == 0) {
        shader_source.append(line + "\n");
        shader_source.append(shader_constants_);
      } else if (line.compare(0, 8, "#include") == 0) {
        shader_source.append(handleInclude(line));
      } else {
        shader_source.append(line + "\n");
      }
    }
    shader_file.close();
    return shader_source;
  } catch (std::ifstream::failure& e) {
    glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION,
                         GL_DEBUG_TYPE_ERROR,
                         0,
                         GL_DEBUG_SEVERITY_HIGH,
                         -1,
                         std::format("(ShaderProgram::Stages::loadShaderSource): Failed to read file from path '{}'. "
                                     "Reason: '{}'",
                                     source_dir_ + filename,
                                     e.what()).c_str());
    return "";
  }
}

std::string ShaderProgram::Stages::handleInclude(const std::string& include_line) {
  const std::string filename {include_line.substr(10, include_line.size() - 11)};
  if (include_cache_.try_emplace(filename, "").second) {
    include_cache_.insert_or_assign(filename, loadShaderSource(filename));
  }
  return include_cache_[filename];
}

ShaderProgram::ShaderProgram(const Stages& stages) {
  program_id_ = glCreateProgram();
  const std::array shader_ids {
    compileShader(stages.vertex_shader_source_, GL_VERTEX_SHADER),
    compileShader(stages.tessellation_control_shader_source_, GL_TESS_CONTROL_SHADER),
    compileShader(stages.tessellation_evaluation_shader_source_, GL_TESS_EVALUATION_SHADER),
    compileShader(stages.geometry_shader_source_, GL_GEOMETRY_SHADER),
    compileShader(stages.fragment_shader_source_, GL_FRAGMENT_SHADER),
    compileShader(stages.compute_shader_source_, GL_COMPUTE_SHADER)
  };
  for (const GLuint shader_id : shader_ids) {
    if (shader_id) glAttachShader(program_id_, shader_id);
  }
  glLinkProgram(program_id_);
  checkCompileOrLinkErrors(program_id_, GL_SHADER);

  // once the program is linked, the shader objects themselves are no longer needed
  for (const GLuint shader_id : shader_ids) {
    if (shader_id) {
      glDetachShader(program_id_, shader_id);
      glDeleteShader(shader_id);
    }
  }
}

GLuint ShaderProgram::compileShader(const std::string& shader_string, const GLenum shader_type) {
  GLuint shader_id {0};
  if (!shader_string.empty()) {
    shader_id = glCreateShader(shader_type);
    const char* shader_c_string {shader_string.c_str()};
    glShaderSource(shader_id, 1, &shader_c_string, nullptr);
    glCompileShader(shader_id);
    checkCompileOrLinkErrors(shader_id, shader_type);
  }
  return shader_id;
}

void ShaderProgram::checkCompileOrLinkErrors(const GLuint program_or_shader, const GLenum program_or_shader_type) {
  GLint success;
  std::array<GLchar, MAX_ERROR_LENGTH> info_log {};
  if (program_or_shader_type == GL_SHADER) {
    glGetProgramiv(program_or_shader, GL_LINK_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(program_or_shader, MAX_ERROR_LENGTH, nullptr, info_log.data());
      glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION,
                           GL_DEBUG_TYPE_ERROR,
                           program_or_shader,
                           GL_DEBUG_SEVERITY_HIGH,
                           -1,
                           std::format("Shader program linking error(s):\n{}", info_log.data()).c_str());
    }
  } else {
    glGetShaderiv(program_or_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(program_or_shader, MAX_ERROR_LENGTH, nullptr, info_log.data());
      std::string type;
      switch (program_or_shader_type) {
        case GL_VERTEX_SHADER:
          type = "Vertex";
          break;
        case GL_TESS_CONTROL_SHADER:
          type = "Tesselation Control";
          break;
        case GL_TESS_EVALUATION_SHADER:
          type = "Tesselation Evaluation";
          break;
        case GL_GEOMETRY_SHADER:
          type = "Geometry";
          break;
        case GL_FRAGMENT_SHADER:
          type = "Fragment";
          break;
        case GL_COMPUTE_SHADER:
          type = "Compute";
          break;
        default:
          type = "Unknown";
          break;
      }
      glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION,
                           GL_DEBUG_TYPE_ERROR,
                           program_or_shader,
                           GL_DEBUG_SEVERITY_MEDIUM,
                           -1,
                           std::format("{} shader compile error(s):\n{}", type, info_log.data()).c_str());
    }
  }
}
