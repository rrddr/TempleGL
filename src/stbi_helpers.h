#ifndef TEMPLEGL_SRC_STBI_HELPERS_H_
#define TEMPLEGL_SRC_STBI_HELPERS_H_

#include "opengl_wrappers.h"

#include <string>

/**
 * Collects helper functions that deal with the stb_image library, which we use for loading textures from files.
 */
namespace help {
  struct StbiDeleter {
    void operator()(unsigned char* data);
  };

  /**
   * Loads an image file and passes it to glTextureSubImage3D.
   *
   * @param path    Path to the image file. Should be RGB or RGBA (although 4th channel will be ignored), with width
   *                and height matching the 3rd and 4th arguments. Otherwise, no data will be loaded.
   * @param texture The 3D texture in which data should be placed.
   * @param layer   The layer (z_offset) which should be filled.
   * @param width   The width of the texture in pixels.
   * @param height  The height of the texture in pixels.
   */
  void fill3DTextureLayer(const std::string& path,
                          const wrap::Texture& texture,
                          GLint layer,
                          GLsizei width,
                          GLsizei height);
}
#endif //TEMPLEGL_SRC_STBI_HELPERS_H_
