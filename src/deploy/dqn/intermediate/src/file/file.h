#pragma once

#include <iostream>
#include <thread>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#define NUMACT 3
#define SLEEPTIME 1

struct Action{
    // defined by OpenROAD, no need to change
    int a;
    float b;
    float c;
};

typedef struct Action action;
typedef action* ptrAction;
class File{

public:

    int envCount;       // C++ environment flag status
    int agentCount;     // C++ agent flag status

    File();
    void getAction(ptrAction act, std::string filename);
    void updateDumpFile(int drc, std::string filename);

};