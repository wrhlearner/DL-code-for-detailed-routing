#include<iostream>
#include "test.h"

int main(int argc, char* argv[]){

    double x[3][2] = {{0, 1}, {2, 3}, {4, 5}};
    double* ptr[3];

    for(auto i: {0, 1, 2}){
        ptr[i] = x[i];
        std::cout << "line " << i << ": " << *ptr[i] << std::endl;
    }
    
    helloWorld(ptr);

    // print value after changing them within function
    std::cout << "value after processing: \n";

    for(auto i: {0, 1, 2}){
        for(auto j: {0, 1})
            std::cout << ptr[i][j] << ", ";
        std::cout << std::endl;
    }
    
    return 0;
}