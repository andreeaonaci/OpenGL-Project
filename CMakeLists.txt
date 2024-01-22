cmake_minimum_required(VERSION 3.27)
project(ProjectGP)

set(CMAKE_CXX_STANDARD 14)

# Add the GLFW subdirectory
add_subdirectory("glfw-3.3.8" ${CMAKE_CURRENT_BINARY_DIR}/glfw)

# Add the GLEW subdirectory
add_subdirectory("glew-2.1.0" ${CMAKE_CURRENT_BINARY_DIR}/glew)

# No need to add GLM as a subdirectory, just specify its include directory
# If you have glm installed system-wide, you can use find_package instead
include_directories("glm")

# Specify the path to your "GL" directory containing glew.h
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/GL")

# Define the source files
set(SOURCE_FILES
        Camera.cpp
        Camera.hpp
        main.cpp
        Mesh.cpp
        Mesh.hpp
        Model3D.cpp
        Model3D.hpp
        Shader.cpp
        Shader.hpp
        SkyBox.cpp
        SkyBox.hpp
        stb_image.cpp
        stb_image.h
        tiny_obj_loader.cpp
        tiny_obj_loader.h)

# Create the executable and link it with the necessary libraries
add_executable(ProjectGP ${SOURCE_FILES})
target_include_directories(ProjectGP PRIVATE ${GLFW_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS})
target_link_libraries(ProjectGP PRIVATE ${GLFW_LIBRARIES} ${GLEW_LIBRARIES})
