#include "shader_program.h"

#include <fstream>
#include <format>
#include <array>

ShaderProgram::Stages& ShaderProgram::Stages::vertex(const std::string& shader_path) {
  vertex_shader_source = loadShaderSource(shader_path);
  return *this;
}
ShaderProgram::Stages& ShaderProgram::Stages::tessellationControl(const std::string& shader_path) {
  tessellation_control_shader_source = loadShaderSource(shader_path);
  return *this;
}
ShaderProgram::Stages& ShaderProgram::Stages::tessellationEvaluation(const std::string& shader_path) {
  tessellation_evaluation_shader_source = loadShaderSource(shader_path);
  return *this;
}
ShaderProgram::Stages& ShaderProgram::Stages::geometry(const std::string& shader_path) {
  geometry_shader_source = loadShaderSource(shader_path);
  return *this;
}
ShaderProgram::Stages& ShaderProgram::Stages::fragment(const std::string& shader_path) {
  fragment_shader_source = loadShaderSource(shader_path);
  return *this;
}
ShaderProgram::Stages& ShaderProgram::Stages::compute(const std::string& shader_path) {
  compute_shader_source = loadShaderSource(shader_path);
  return *this;
}

std::string ShaderProgram::Stages::loadShaderSource(const std::string& shader_path) {
  std::ifstream shader_file;
  shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  std::string shader_source;
  try {
    shader_file.open(shader_path);
    shader_source.assign(std::istreambuf_iterator<char>(shader_file), std::istreambuf_iterator<char>());
    shader_file.close();
    return shader_source;
  }
  catch (std::ifstream::failure& e) {
    glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_ERROR, 0, GL_DEBUG_SEVERITY_HIGH, -1,
                         std::format("(ShaderProgram::Stages::loadShaderSource): Failed to read file from path '{}'",
                                     shader_path).c_str());
    return "";
  }
}

ShaderProgram::ShaderProgram(const ShaderProgram::Stages& stages) {
  program_id = glCreateProgram();
  const std::array<GLuint, 6> shader_ids {
      compileShader(stages.vertex_shader_source, GL_VERTEX_SHADER),
      compileShader(stages.tessellation_control_shader_source, GL_TESS_CONTROL_SHADER),
      compileShader(stages.tessellation_evaluation_shader_source, GL_TESS_EVALUATION_SHADER),
      compileShader(stages.geometry_shader_source, GL_GEOMETRY_SHADER),
      compileShader(stages.fragment_shader_source, GL_FRAGMENT_SHADER),
      compileShader(stages.compute_shader_source, GL_COMPUTE_SHADER)
  };
  for (const GLuint& shader_id : shader_ids) {
    if (shader_id) glAttachShader(program_id, shader_id);
  }
  glLinkProgram(program_id);
  checkCompileOrLinkErrors(program_id, GL_SHADER);

  // once the program is linked, the shader objects themselves are no longer needed
  for (const GLuint& shader_id : shader_ids) {
    if (shader_id) {
      glDetachShader(program_id, shader_id);
      glDeleteShader(shader_id);
    }
  }
}

GLuint ShaderProgram::compileShader(const std::string& shader_string, GLenum shader_type) {
  GLuint shader_id {0};
  if (!shader_string.empty()) {
    shader_id = glCreateShader(shader_type);
    const char* shader_c_string {shader_string.c_str()};
    glShaderSource(shader_id, 1, &shader_c_string, nullptr);
    glCompileShader(shader_id);
    ShaderProgram::checkCompileOrLinkErrors(shader_id, shader_type);
  }
  return shader_id;
}

void ShaderProgram::checkCompileOrLinkErrors(GLuint program_or_shader, GLenum program_or_shader_type) {
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