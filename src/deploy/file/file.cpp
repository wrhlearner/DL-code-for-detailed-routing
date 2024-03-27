#include "file.h"

File::File(){
    // initialize C++ environment and agent flag status
    envCount = -1;
    agentCount = -1;
}

void File::getAction(ptrAction act, std::string filename){
    // agent -> action -> dump File -> action -> environment
    // wait for dump File update and detect agent status change
    int fileAgentCount;     // agentCount in dump file
    bool flag = false;      // check whether dump file has been updated
    int actionValue;        // stores the index of action space
    std::string firstLine;
    // if dump file exists and agentCount in dump file 
    // has been updated, then proceed to get action
    // otherwise, wait for updating
    while(!flag){
        if((std::filesystem::exists(filename) && std::filesystem::is_regular_file(filename))){
            // read and parse dump file
            std::ifstream dumpFile(filename);
            // check if the file is opened successfully
            if(!dumpFile.is_open()){
                std::cerr << "C++ runtime fails to open dump file " << filename << std::endl;
                return;
            }
            // read most updated agentCount value
            std::getline(dumpFile, firstLine);
            std::istringstream iss(firstLine);
            iss >> fileAgentCount;
            iss >> actionValue;
            if(fileAgentCount > File::agentCount){
                // update and return new action
                act->a = actionValue;
                // update C++ agent status flag
                File::agentCount = fileAgentCount;
                flag = true;
            }
            else{   // dump file hasn't been updated, wait for some time to read again
                std::this_thread::sleep_for(std::chrono::seconds(SLEEPTIME));
            }
            dumpFile.close();
        }
    }
    // report value
    std::cout << "getAction\nagentCount: " << this->agentCount << ", action: " << act->a << "\n\n";
};

void File::updateDumpFile(int drc, std::string filename){
    // environment -> state -> dump File -> state -> agent
    // open dump File, update dump file envCount state
    std::ifstream dumpFile(filename);
    // check if dump file opens successfully
    if(!dumpFile.is_open()){
        std::cerr << "C++ runtime fails to open dump file " << filename << std::endl;
        return;
    }
    // read in file line by line
    std::vector<std::string> lines;
    std::string line;
    std::string firstLine;
    // update envCount and drc to dump file by updating the first line
    std::getline(dumpFile, firstLine);
    std::stringstream ss(firstLine);
    int fileAgentCount, fileAction;
    ss >> fileAgentCount;
    // ss.ignore();
    ss >> fileAction;
    firstLine = std::to_string(fileAgentCount) + " " + std::to_string(fileAction) + " " \
            + std::to_string(++this->envCount) + " " + std::to_string(drc);
    // get the rest of the file
    while(std::getline(dumpFile, line)){
        lines.push_back(line);
    }
    // close the input file
    dumpFile.close();
    // open dump file for writing, clearing previous contents
    std::ofstream newDumpFile(filename, std::ios::out | std::ios::trunc);
    // check file opens successfully
    if(!newDumpFile.is_open()){
        std::cerr << "C++ runtime fails to update dump file " << filename << std::endl;
        return;
    }  
    // write modified lines to the file
    newDumpFile << firstLine << std::endl;
    for(const std::string& line: lines)
        newDumpFile << line << std::endl;
    // close the updated dump file
    newDumpFile.close();
    // report value
    std::cout << "updateDumpFile\nfile::envCount: " << this->envCount << ", drc: " << drc << "\n\n";
};