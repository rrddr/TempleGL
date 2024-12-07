#ifndef TEMPLEGL_SRC_SHADER_PROGRAM_H_
#define TEMPLEGL_SRC_SHADER_PROGRAM_H_

#include <glad/glad.h>
#include <string>

/**
 * Used to create and interact with an OpenGL shader program.
 */
class ShaderProgram {
 public:
  /**
   * Stores GLSL source code for each shader stage used in a shader program.
   */
  class Stages {
   public:
    std::string vertex_shader_source;
    std::string tessellation_control_shader_source;
    std::string tessellation_evaluation_shader_source;
    std::string geometry_shader_source;
    std::string fragment_shader_source;
    std::string compute_shader_source;

    // Utility methods to load source from file. Should be chained
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
   * Constructor compiles and links all shader stages. Empty stages are skipped.
   */
  explicit ShaderProgram(const Stages& stages);
  ~ShaderProgram() { glDeleteProgram(program_id); }

  inline void use() const { glUseProgram(program_id); }

 private:
  GLuint program_id;

  /**
   * Compiles one stage of a shader program.
   *
   * @param shader_string   The GLSL source code to be compiled.
   * @param shader_type     One of GL_VERTEX_SHADER, GL_TESS_CONTROL_SHADER, GL_TESS_EVALUATION_SHADER,
   *                        GL_GEOMETRY_SHADER, GL_FRAGMENT_SHADER, or GL_COMPUTE_SHADER.
   *
   * @returns   The id of the generated shader object.
   */
  [[nodiscard("Return value must be passed to glDeleteShader() for proper resource de-allocation.")]]
  static GLuint compileShader(const std::string& shader_string, GLenum shader_type);

  /**
   * Sends an OpenGL debug message event if any errors occurred during most recent glCompileShader() or glLinkProgram()
   * call. File read and linking errors are High severity, compile errors are Medium severity (since they are often
   * duplicated in the resulting linking error message).
   *
   * @param program_or_shader       The id of either the shader object, or the program being inspected.
   * @param program_or_shader_type  When inspecting a shader, this should be the corresponding shader type.
   *                                When inspecting a program, this should be GL_SHADER.
   */
  static void checkCompileOrLinkErrors(GLuint program_or_shader, GLenum program_or_shader_type);

  static constexpr GLsizei MAX_ERROR_LENGTH {1024};
};
#endif //TEMPLEGL_SRC_SHADER_PROGRAM_H_
