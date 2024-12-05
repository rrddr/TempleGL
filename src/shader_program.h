#ifndef TEMPLEGL_SRC_SHADER_PROGRAM_H_
#define TEMPLEGL_SRC_SHADER_PROGRAM_H_

#include <glad/glad.h>
#include <string>
#include <map>

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
  explicit Stages(std::string source_directory) : source_dir_(std::move(source_directory)) {}

  std::string vertex_shader_source_;
  std::string tessellation_control_shader_source_;
  std::string tessellation_evaluation_shader_source_;
  std::string geometry_shader_source_;
  std::string fragment_shader_source_;
  std::string compute_shader_source_;

  // Utility methods to load source from file. Should be chained
  Stages& vertex(const std::string& filename);
  Stages& tessellationControl(const std::string& filename);
  Stages& tessellationEvaluation(const std::string& filename);
  Stages& geometry(const std::string& filename);
  Stages& fragment(const std::string& filename);
  Stages& compute(const std::string& filename);

 private:
  std::string source_dir_;
  inline static std::map<std::string, std::string> include_cache_ {};

  std::string loadShaderSource(const std::string& filename);
  std::string handleInclude(const std::string& include_line);
 };

 /**
  * Constructor compiles and links all shader stages. Empty stages are skipped.
  */
 explicit ShaderProgram(const Stages& stages);
 ~ShaderProgram() { glDeleteProgram(program_id_); }
 void use() const { glUseProgram(program_id_); }

private:
 GLuint program_id_;

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
