#include<iostream>
#include "env.h"

// private functions


// public functions
Env::Env(int windowSize, int inputSize, double drcmax){

};

Env::~Env(){

};

void Env::reset(std::map<std::string, int>& info, double* observation[]){

};

void Env::step(int action, double* observation[], double* reward, bool* terminated, bool* truncated, std::map<std::string, int>& info){

};

int Env::selectAction(double* observation[]){

};

void Env::getAction(){
    // print action space within environment

};
