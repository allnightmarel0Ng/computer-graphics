cmake_minimum_required(VERSION 3.25)
project(lab1)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)

FetchContent_Declare(
        sfml
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 2.5.1
)

FetchContent_MakeAvailable(sfml)

find_package(OpenGL REQUIRED)

add_executable(lab1
        src/main.cpp)

target_link_libraries(lab1 sfml-graphics sfml-window sfml-system ${OPENGL_LIBRARIES})