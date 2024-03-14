#include <iostream>
#include <map>
#include <string>

// TODO 3: include dependencies
#include "env.h"

int main(int argc, const char* argv[]) {
    if (argc != 2) {
        std::cout << "argc: " << argc << std::endl;
        return -1;
    }
    
    // TODO: instantiate environment
    int windowSize = 2;
    int inputSize = 9;
    double drcmax = 1e6;

    Env env(windowSize, inputSize, drcmax);
    env.getAction();

    // TODO: reset env
    std::map<std::string, int> info;
    double* observation[WINDOW];

    env.reset(info, observation);
    // TODO: get action from policy net
    int action;

    action = env.selectAction(observation);
    
    double* reward;
    bool* terminated;
    bool* truncated;

    env.step(action, observation, reward, terminated, truncated, info);
    // TODO: run a test loop until succeed or fail
    int count = 0;

    while((!(*terminated))&&(!(*truncated))){
        // print iteration information
        std::cout << "Iteration " << count++ << ": action " << action << ", DRC = " << int(drcmax * observation[ROWNUM - 1][ITEMNUM - 1]) << std::endl;
        // select action for next iteration
        action = env.selectAction(observation);
        env.step(action, observation, reward, terminated, truncated, info);
    }

    if(*terminated){
        std::cout << "Terminated!\n";
        std::cout << "Iteration " << count++ << ": action " << action << ", DRC = " << int(drcmax * observation[ROWNUM - 1][ITEMNUM - 1]) << std::endl;
    }
    if(*truncated){
        std::cout << "Truncated!\n";
        std::cout << "Iteration " << count++ << ": action " << action << ", DRC = " << int(drcmax * observation[ROWNUM - 1][ITEMNUM - 1]) << std::endl;
    }

    return 0;
}