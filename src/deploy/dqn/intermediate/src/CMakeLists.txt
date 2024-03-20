cmake_minimum_required(VERSION 3.10)

project(deploy VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# add environment class
add_subdirectory(file)

# TODO 1: add executable using the specified source file
add_executable(deploy helloWorld.cpp)

# link the library to executable
target_link_libraries(deploy file)

# add env to DQN's include directories
# ${PROJECT_SOURCE_DIR} is the directory to current CMakeLists.txt file
# ${PROJECT_BINARY_DIR} is the binary tree to the search path for include files
target_include_directories(deploy PUBLIC 
    "${PROJECT_SOURCE_DIR}/file"
    "${PROJECT_BINARY_DIR}"
)
