#ifndef TEMPLEGL_SRC_SHADER_H_
#define TEMPLEGL_SRC_SHADER_H_

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>

/**
 * Used to create and interact with an OpenGL Shader Program.
 */
class ShaderProgram {
 public:
  /**
   * Stores GLSL source code for each shader stage used in this program.
   */
  struct Stages {
   public:
    std::string vertex_shader_source;
    std::string tessellation_control_shader_source;
    std::string tessellation_evaluation_shader_source;
    std::string geometry_shader_source;
    std::string fragment_shader_source;
    std::string compute_shader_source;

    // Utility methods to load source from file. Can (and should) be chained
    Stages& vertex(const std::string& shader_path);
    Stages& tessellationControl(const std::string& shader_path);
    Stages& tessellationEvaluation(const std::string& shader_path);
    Stages& geometry(const std::string& shader_path);
    Stages& fragment(const std::string& shader_path);
    Stages& compute(const std::string& shader_path);

   private:
    static std::string loadShaderSource(const std::string& shader_path);
  };

  /**
   * Constructor compiles and links all shader stages.
   *
   * @param stages  Contains the source code for each stage used in this program. Empty stages will be skipped.
   */
  explicit ShaderProgram(const Stages& stages);
  ~ShaderProgram() { glDeleteProgram(program_id); }

  /// Utility methods for interacting with the Shader Program
  inline void use() const { glUseProgram(program_id); }
  inline void setBool(const std::string& name, bool value) const {
    glUniform1i(glGetUniformLocation(program_id, name.c_str()), (int) value);
  }
  inline void setInt(const std::string& name, int value) const {
    glUniform1i(glGetUniformLocation(program_id, name.c_str()), value);
  }
  inline void setFloat(const std::string& name, float value) const {
    glUniform1f(glGetUniformLocation(program_id, name.c_str()), value);
  }
  inline void setDouble(const std::string& name, double value) const {
    glUniform1d(glGetUniformLocation(program_id, name.c_str()), value);
  }
  inline void setVec2(const std::string& name, const glm::vec2& value) const {
    glUniform2fv(glGetUniformLocation(program_id, name.c_str()), 1, &value[0]);
  }
  inline void setVec2(const std::string& name, float x, float y) const {
    glUniform2f(glGetUniformLocation(program_id, name.c_str()), x, y);
  }
  inline void setVec3(const std::string& name, const glm::vec3& value) const {
    glUniform3fv(glGetUniformLocation(program_id, name.c_str()), 1, &value[0]);
  }
  inline void setVec3(const std::string& name, float x, float y, float z) const {
    glUniform3f(glGetUniformLocation(program_id, name.c_str()), x, y, z);
  }
  inline void setVec4(const std::string& name, const glm::vec4& value) const {
    glUniform4fv(glGetUniformLocation(program_id, name.c_str()), 1, &value[0]);
  }
  inline void setVec4(const std::string& name, float x, float y, float z, float w) const {
    glUniform4f(glGetUniformLocation(program_id, name.c_str()), x, y, z, w);
  }
  inline void setMat2(const std::string& name, const glm::mat2& mat) const {
    glUniformMatrix2fv(glGetUniformLocation(program_id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
  }
  inline void setMat3(const std::string& name, const glm::mat3& mat) const {
    glUniformMatrix3fv(glGetUniformLocation(program_id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
  }
  inline void setMat4(const std::string& name, const glm::mat4& mat) const {
    glUniformMatrix4fv(glGetUniformLocation(program_id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
  }

 private:
  GLuint program_id;

  /**
   * Compiles one stage of a shader program, and returns the id of the generated shader object.
   *
   * @param shader_string   The GLSL source code to be compiled.
   * @param shader_type     One of GL_VERTEX_SHADER, GL_TESS_CONTROL_SHADER, GL_TESS_EVALUATION_SHADER,
   *                        GL_GEOMETRY_SHADER, GL_FRAGMENT_SHADER, or GL_COMPUTE_SHADER.
   *
   * @returns   The id of the generated shader object. The object must be manually deleted using { glDeleteShader }.
   */
  [[nodiscard]] static GLuint compileShader(const std::string& shader_string, GLenum shader_type) ;

  /**
   * Used to check for errors after compiling or linking. Outputs a message to stdout if any errors occurred.
   * This function is redundant in Debug mode, but we want to know about these errors even when Debug mode is off.
   *
   * @param program_or_shader       The id of either the shader object, or the program being inspected.
   * @param program_or_shader_type  When inspecting a shader, this should be the corresponding shader type.
   *                                When inspecting a program, this should be GL_SHADER.
   */
  static void checkCompileOrLinkErrors(GLuint program_or_shader, GLenum program_or_shader_type) ;
};
#endif //TEMPLEGL_SRC_SHADER_H_
