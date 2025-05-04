# CMakeLists.txt
cmake_minimum_required(VERSION 3.13)
project(Gstar VERSION 1.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(SOURCES
    src/main.cpp
    # add other .cpp/.h files here (network is header-only)
)

# Set default build type if none was specified
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
endif()

find_package(raylib REQUIRED) 

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)
target_include_directories(${PROJECT_NAME} PRIVATE include)

# install target (for AppImage/App bundle packaging)
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION usr/bin)

# Install resources next to the binary for runtime asset loading
install(DIRECTORY resources DESTINATION usr/bin)

# CPack configuration to enable creation of AppImage (Linux) and .dmg (macOS)
set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
include(InstallRequiredSystemLibraries)
include(CPack)