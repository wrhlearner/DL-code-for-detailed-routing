#include "file.cpp"
#include <iostream>
#include <string>

int step(action act){
    // make one step
    // defined by OpenROAD, no need to change
    return act->a + 1;
};

int main(int argc, char* argv[]){
    
    // TODO 1: transfer initial data of environment for initialization
    std::string initFilename = "../data/initFile.txt";
    std::string dumpFilename = "../data/dumpFile.txt";
    action actionSpace[NUMACT];

    file::initFile(initFilename, actionSpace);

    // TODO 2: start loop
    int drc = 1;
    action act;

    do{
        // get action for current iteration
        file::getAction(act, dumpFilename);
        // make one step, and update dump file
        drc = step(act);
        file::updateDumpFile(drc, dumpFilename);
        std::cout << "iteration " << file::envCount << " finished!" << std::endl;

    }while(drc);    // check

    return 0;
}