cmake_minimum_required(VERSION 3.30)
project(TempleGL)
set(CMAKE_CXX_STANDARD 23)

add_executable(TempleGL src/main.cpp
        src/camera.h
        src/camera.cpp
        src/shader_program.h
        src/shader_program.cpp
        src/model.h
        src/model.cpp
        src/initializer.h
        src/initializer.cpp
        src/renderer.h
        src/renderer.cpp
        src/opengl_wrappers.h
        src/skybox.cpp
        src/skybox.h
        src/stbi_helpers.h
        src/stbi_helpers.cpp
)

find_package(glfw3 CONFIG REQUIRED)
message("Linking libraries: glfw")
target_link_libraries(TempleGL glfw)

find_package(glad CONFIG REQUIRED)
message("Linking libraries: glad")
target_link_libraries(TempleGL glad::glad)

find_package(OpenGL REQUIRED)
message("Linking libraries: ${OPENGL_gl_LIBRARY}")
target_link_libraries(TempleGL ${OPENGL_gl_LIBRARY})

find_package(glm CONFIG REQUIRED)
message("Linking libraries: glm")
target_link_libraries(TempleGL glm::glm)

find_package(assimp CONFIGS REQUIRED)
message("Linking libraries: assimp")
target_link_libraries(TempleGL assimp::assimp)

find_package(yaml-cpp CONFIG REQUIRED)
message("Linking libraries: yaml-cpp")
target_link_libraries(TempleGL yaml-cpp::yaml-cpp)

find_package(Stb REQUIRED)
message("Include directory: ${Stb_INCLUDE_DIR}")
target_include_directories(TempleGL PRIVATE ${Stb_INCLUDE_DIR})
