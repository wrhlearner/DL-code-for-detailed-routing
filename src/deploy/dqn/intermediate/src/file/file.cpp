#include "file.h"

namespace file{

    int envCount = -1;        // environment status flag for data synchronization
    int agentCount = -1;      // agent status flag for data synchronization

    void initFile(std::string filename, action act[NUMACT]){
        // environment -> action space -> init file
        // create init file if file doesn't exists
        
        // write action space data into init file

    };

    void getAction(action act, std::string filename, int agent){
        // agent -> action -> dump file -> action -> environment
        // open dump file and detect agent status change
        
        // update and return new action
        
    };

    void updateDumpFile(int state, std::string filename){
        // environment -> state -> dump file -> state -> agent
        // open dump file, update state, and update environment status flag when finish

        
    };

};