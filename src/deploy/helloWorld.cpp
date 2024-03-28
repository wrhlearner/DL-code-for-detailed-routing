// debug C++ (CMake project) https://www.youtube.com/watch?v=Rfj40xW9q6w
// debug launch.json settings https://vector-of-bool.github.io/docs/vscode-cmake-tools/debugging.html
#include <iostream>
#include <string>
#include "file.cpp"

int step(bool init, int drc){
    // make one step
    // defined by OpenROAD, no need to change
    // this is a fake testing case
    if(init)
        return drc;
    else{
        if(drc!=0){
            return int(drc/2);
        }
        else
            return drc;
    }
};

int main(int argc, char* argv[]){
    
    // TODO 1: transfer initial data of environment for initialization
    std::string dumpFilename = "/home/jborg/Data/Research/DLPnR/DL-code-for-detailed-routing/src/deploy/data/dumpFile.txt";

    File fileAPI;

    // TODO 2: start loop
    int drc;
    bool init = true;
    ptrAction act = new action;

    // report initial status
    std::cout << "initial flag values\nagentCount: " << fileAPI.agentCount << ", envCount: " << fileAPI.envCount << "\n\n";

    do{
        // get action for current iteration
        fileAPI.getAction(act, dumpFilename);
        // make one step, and update dump File
        if(init){
            drc = step(init, INITDRC);
            init = false;
        }
        else
            drc = step(init, drc);
        fileAPI.updateDumpFile(drc, dumpFilename);
        // report iteration info
        std::cout << "iteration " << fileAPI.envCount << " finished!\n\n";

    }while(drc);    // check

    return 0;
}