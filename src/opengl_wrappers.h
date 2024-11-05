#ifndef TEMPLEGL_SRC_OPENGL_WRAPPERS_H_
#define TEMPLEGL_SRC_OPENGL_WRAPPERS_H_

#include <glad/glad.h>

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
