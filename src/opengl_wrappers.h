#ifndef TEMPLEGL_SRC_OPENGL_WRAPPERS_H_
#define TEMPLEGL_SRC_OPENGL_WRAPPERS_H_

#include <glad/glad.h>

/**
 * By wrapping the id of a created OpenGL object in the appropriate struct below, we can be sure it will be deleted
 * when it goes out of scope. Additionally, a debug message is sent when this happens, which is helpful for finding
 * the cause of otherwise very hard to diagnose bugs.
 *
 * Note: use std::unique_ptr when placing these objects inside containers to prevent undesired destructor calls
 * (e.g. on std::vector reallocation).
 */
namespace wrap {
  struct Texture {
    GLuint id;
    ~Texture() {
      glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_OTHER, id, GL_DEBUG_SEVERITY_MEDIUM, -1,
                           "Deleting Texture");
      glDeleteTextures(1, &id);
    }
  };
  struct Buffer {
    GLuint id;
    ~Buffer() {
      glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_OTHER, id, GL_DEBUG_SEVERITY_MEDIUM, -1,
                           "Deleting Buffer");
      glDeleteBuffers(1, &id);
    }
  };
  struct VertexArray {
    GLuint id;
    ~VertexArray() {
      glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_OTHER, id, GL_DEBUG_SEVERITY_MEDIUM, -1,
                           "Deleting Vertex Array");
      glDeleteVertexArrays(1, &id);
    }
  };
  struct Framebuffer {
    GLuint id;
    ~Framebuffer() {
      glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_OTHER, id, GL_DEBUG_SEVERITY_MEDIUM, -1,
                           "Deleting Framebuffer");
      glDeleteFramebuffers(1, &id);
    }
  };
  struct Renderbuffer {
    GLuint id;
    ~Renderbuffer() {
      glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_OTHER, id, GL_DEBUG_SEVERITY_MEDIUM, -1,
                           "Deleting Renderbuffer");
      glDeleteRenderbuffers(1, &id);
    }
  };
}
#endif //TEMPLEGL_SRC_OPENGL_WRAPPERS_H_
