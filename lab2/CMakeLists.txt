cmake_minimum_required(VERSION 3.10)
project(lab2)

set(CMAKE_CXX_STANDARD 14)

find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

set(SOURCE_FILES src/main.cpp)

add_executable(lab2 src/main.cpp)

target_link_libraries(lab2
    sfml-graphics
    sfml-window
    sfml-system
)

target_include_directories(lab2 PRIVATE
    ${SFML_INCLUDE_DIR}
)