cmake_minimum_required(VERSION 3.27)
project(TempleGL)

set(CMAKE_CXX_STANDARD 23)

add_executable(TempleGL src/main.cpp)

find_package(glfw3 CONFIG REQUIRED)
message("Linking libraries: glfw ")
target_link_libraries(TempleGL glfw)

find_package(glad CONFIG REQUIRED)
message("Linking libraries: glad")
target_link_libraries(TempleGL glad::glad)

# Link OpenGL
find_package(OpenGL REQUIRED)
message("Linking libraries: ${OPENGL_gl_LIBRARY}")
target_link_libraries(TempleGL ${OPENGL_gl_LIBRARY})

find_package(glm CONFIG REQUIRED)
message("Linking libraries: glm")
target_link_libraries(TempleGL glm::glm)

find_package(assimp CONFIGS REQUIRED)
message("Linking libraries: assimp")
target_link_libraries(TempleGL assimp::assimp)

find_package(Stb REQUIRED)
message("Include directory: ${Stb_INCLUDE_DIR}")
target_include_directories(TempleGL PRIVATE ${Stb_INCLUDE_DIR})
