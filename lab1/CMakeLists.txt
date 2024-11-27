cmake_minimum_required(VERSION 3.25)
project(lab1)

set(CMAKE_CXX_STANDARD 14)

find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(lab1
        src/main.cpp)

target_link_libraries(lab1 sfml-graphics sfml-window sfml-system ${OPENGL_LIBRARIES})