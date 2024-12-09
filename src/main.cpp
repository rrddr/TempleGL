#include "renderer.h"

#include <iostream>

int main() {
  Renderer renderer {};
  try { renderer.run(); } catch (std::runtime_error& e) {
    std::cerr << std::endl << "FATAL ERROR: " << typeid(e).name() << std::endl << e.what() << std::endl;
    glfwTerminate();
    return -1;
  }
  return 0;
}
