cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(lstm)

find_package(Torch REQUIRED)

add_executable(lstm lstm.cpp)
target_link_libraries(lstm "${TORCH_LIBRARIES}")
#set_property(TARGET lstm PROPERTY CXX_STANDARD 17)
