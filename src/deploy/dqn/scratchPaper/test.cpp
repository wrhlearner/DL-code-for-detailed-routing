#include<iostream>
#include "test.h"

void helloWorld(double* ptr[]){
    // print original value
    std::cout << "original value: \n";

    for(auto i: {0, 1, 2}){
        for(auto j: {0, 1})
            std::cout << ptr[i][j] << ", ";
        std::cout << std::endl;
    }

    // change value within function    
    for(auto i: {0, 1, 2})
        for(auto j: {0, 1})
            ptr[i][j] += 1;

}