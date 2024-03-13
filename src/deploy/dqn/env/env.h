#include<iostream>
#include <torch/script.h>

#define ITEMNUM 9

// environment implementaion
class Env{
public:
    Env(double (*action)[ITEMNUM], int window_size, bool render_mode, double drcmax, int input_size);
    void reset(int seed);
    void step(double (*action)[ITEMNUM]);

private:
    double size;
    double maxdrc;
    int window_size;
    int input_size;
    int index;
    double (*actions)[ITEMNUM];
    double (*observations)[ITEMNUM];

    void preprocessing();
    double* action_to_setting(double (*action)[ITEMNUM]);
    double* get_obs(double (*action)[ITEMNUM]);
    std::string* get_info();
}

// select action
int select_action(double (*observation), )