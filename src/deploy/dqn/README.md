# **README**

[toc]

## **How to build C++ code?**

- finish TODO 1 to TODO 3 in ./deploy/dqn/env/CMakeLists.txt, ./deploy/dqn/CMakeLists.txt ./deploy/dqn/main.cpp

- open a terminal, cd into ./deploy/dqn/build

- run the following command

  ```cmake
  terminal$ cmake --build .
  ```

> **Note:**
>
> if there is no build directory within ./deploy/dqn, run the following commands to build
>
> ```cmake
> terminal$ cd ./deploy/dqn
> terminal$ mkdir build
> terminal$ cd ./build
> terminal$ cmake ../
> terminal$ cmake --build .
> ```

## **Reference**

[CMake Tutorial](https://cmake.org/cmake/help/latest/guide/tutorial/index.html), Step 1 and Step 2

