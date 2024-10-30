#include "renderer.h"

#include <stdexcept>

int main() {
  Renderer renderer{};
  try {
    renderer.run();
  }
  catch (std::runtime_error& e) {
    return -1;
  }
  return 0;
}
