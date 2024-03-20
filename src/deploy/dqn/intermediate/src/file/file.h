#pragma once

#include <iostream>
#include <string>

#define NUMACT 3

typedef struct Action* action;

struct Action{
    // defined by OpenROAD, no need to change
    int a;
    bool b;
    char c[5];
};

namespace file{

    int count;      // record number of data point for synchronization

    void initFile(std::string filename, action act[]);
    void getAction(action act, std::string filename);
    void updateDumpFile(int drc, std::string filename);

};