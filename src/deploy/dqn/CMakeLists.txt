cmake_minimum_required(VERSION 3.10)

project(DQN VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_PREFIX_PATH /home/jborg/App/libtorch)

# add environment class
add_subdirectory(env)

# add libtorch dependencies
find_package(Torch REQUIRED)

# TODO 2: add executable using the specified source file
add_executable(DQN main.cpp)

# link the library to executable
target_link_libraries(DQN env)
target_link_libraries(DQN "${TORCH_LIBRARIES}")

# add env to DQN's include directories
# ${PROJECT_SOURCE_DIR} is the directory to current CMakeLists.txt file
# ${PROJECT_BINARY_DIR} is the binary tree to the search path for include files
target_include_directories(DQN PUBLIC 
    "${PROJECT_SOURCE_DIR}/env"
    "${PROJECT_BINARY_DIR}"
)
