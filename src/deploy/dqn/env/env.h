#include <iostream>
#include <map>
#include <string>

#define ITEMNUM 9
#define ROWNUM  65
#define WINDOW  2
// environment implementaion
class Env{
private:
    double size;
    double maxdrc;
    int windowSize;
    int inputSize;
    int index;
    
    void preprocessing();
    void action_to_setting(double* action[]);
    void get_obs(double* action[]);
    void get_info(std::string* info);

public:
    double reward;
    bool terminated;
    bool truncated;
    double* actions[ROWNUM];
    double* obs[WINDOW];

    Env(int windowSize, int inputSize, double drcmax);
    ~Env();
    void reset(std::map<std::string, int>& info, double* observation[]);
    void step(int action, double* observation[], double* reward, bool* terminated, bool* truncated, std::map<std::string, int>& info);
    int selectAction(double* observation[]);
    void getAction();
};
