# CMakeLists.txt
cmake_minimum_required(VERSION 3.13)
project(Gstar VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(SOURCES
    src/main.cpp
    src/network.cpp
    # add other .cpp/.h files here
)

find_package(raylib REQUIRED) 

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)

# install target (for AppImage/App bundle packaging)
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION usr/bin)