#pragma once

#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>

#define NUMACT 3

struct Action{
    // defined by OpenROAD, no need to change
    int a;
    bool b;
    std::string c;
};

typedef struct Action action;
typedef action* ptrAction;
class File{

public:

    int envCount;       // C++ environment flag status
    int agentCount;     // C++ agent flag status

    File();
    void initFile(std::string filename, ptrAction act[]);
    void getAction(ptrAction act, std::string filename);
    void updateDumpFile(int drc, std::string filename);

};