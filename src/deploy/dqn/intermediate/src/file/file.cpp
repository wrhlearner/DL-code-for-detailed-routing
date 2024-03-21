#include "file.h"

File::File(){
    // initialize C++ environment and agent flag status
    envCount = -1;
    agentCount = -1;
}

void File::initFile(std::string filename, ptrAction act[NUMACT]){
    // environment -> action space -> init File
    if(!(std::filesystem::exists(filename) && std::filesystem::is_regular_file(filename))){
        // create init File if File doesn't exists
        std::ofstream initFile(filename);
        // write action space data into init File
        if(initFile.is_open()){
            for(int i = 0; i < NUMACT; i++){
                initFile << act[i]->a << "," << act[i]->b << "," << act[i]->c << std::endl;
            }
            initFile.close();
            std::cout<<"Action space has been written into file " << filename << "!\n";
        }
        else{
            std::cerr << "Error: Unable to create or open init file " << filename << std::endl;
        }
    }
    else{
        std::cout << "init file " << filename << " already exists!\n";
    }
};

void File::getAction(ptrAction act, std::string filename){
    // agent -> action -> dump File -> action -> environment
    // wait for dump File update and detect agent status change
    int fileAgentCount;
    // update and return new action

    // update C++ agent status flag
    agentCount = fileAgentCount;
};

void File::updateDumpFile(int state, std::string filename){
    // environment -> state -> dump File -> state -> agent
    // open dump File, update dump file envCount state
    
    // update drc to dump file

    
};