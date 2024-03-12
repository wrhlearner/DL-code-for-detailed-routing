#include<iostream>
#include "env.h"

// public functions
Env::Env(double (*action)[ITEMNUM], int window_size, bool render_mode, double drcmax, int input_size){

}

void Env::reset(int seed){

}

void Env::step(double (*action)[ITEMNUM]){

}

// private functions
void Env::preprocessing(){

}

double* Env::action_to_setting(double (*action)[ITEMNUM]){

}

double* Env::get_obs(double (*action)[ITEMNUM]){

}

std::string* Env::get_info(){

}