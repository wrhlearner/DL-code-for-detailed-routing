#include "file.cpp"
#include <iostream>
#include <string>

int step(ptrAction act){
    // make one step
    // defined by OpenROAD, no need to change
    // this is a fake testing case
    return act->a - 1;
};

int main(int argc, char* argv[]){
    
    // TODO 1: transfer initial data of environment for initialization
    std::string initFilename = "../data/initFile.txt";
    std::string dumpFilename = "../data/dumpFile.txt";
    ptrAction actionSpace[NUMACT];

    File fileAPI;

    // initialize action space
    for(int i = 0; i<NUMACT; i++){
        actionSpace[i] = new action;
        actionSpace[i]->a = i;
        actionSpace[i]->b = true;
        actionSpace[i]->c = "this is a test!";
    }

    fileAPI.initFile(initFilename, actionSpace);

    // TODO 2: start loop
    int drc = 1;
    ptrAction act = new action;

    do{
        // get action for current iteration
        fileAPI.getAction(act, dumpFilename);
        // make one step, and update dump File
        drc = step(act);
        fileAPI.updateDumpFile(drc, dumpFilename);
        std::cout << "iteration " << fileAPI.envCount << " finished!" << std::endl;

    }while(drc);    // check

    return 0;
}