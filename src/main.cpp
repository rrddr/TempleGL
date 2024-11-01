#include "renderer.h"

#include <iostream>
#include <stdexcept>

int main() {
  Renderer renderer{};
  try {
    renderer.run();
  }
  catch (std::runtime_error& e) {
    std::cerr << "FATAL ERROR" << std::endl << e.what() << std::endl;
    glfwTerminate(); // Should be unnecessary, but doesn't hurt to be safe
    return -1;
  }
  return 0;
}
