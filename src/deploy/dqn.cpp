// pytorch C++ API https://pytorch.org/cppdocs/
#include <torch/script.h> // One-stop header.

#include <iostream>
#include <memory>

#include "env.cpp"

// define environment settings
#define WINDOWSIZE  2
#define INPUTSIZE   9
#define DRCMAX      1e6

int main(int argc, const char* argv[]) {
  if (argc != 2) {
    std::cerr << "usage: LSTM ../model/LSTM_hidden10_layer10_window2.pt\n";
    std::cout << "argc: " << argc << std::endl;
    return -1;
  }


  torch::jit::script::Module module;
  try {
    // Deserialize the ScriptModule from a file using torch::jit::load().
    module = torch::jit::load(argv[1]);
  }
  catch (const c10::Error& e) {
    std::cerr << "error loading the model\n";
    return -1;
  }

  std::cout << "ok\n";
  
//   environment implementation
//   python version of test code
// # test loop
// count = 0
// # start trial
// env.reset()
// action = select_test_action(observation, model)
// observation, reward, terminated, truncated, info = env.step(action)
// while not(terminated or truncated):
//     print(f"iteration {count} action {action} DRC = {int(observation[-1, -1] * 1e6)}\n")
//     # start next iteration
//     count += 1
//     action = select_test_action(observation, model)
//     observation, reward, terminated, truncated, info = env.step(action)
    
// print(f"iteration {count} action {action} DRC = {int(observation[-1, -1] * 1e6)}\n")
// env.close()

  // instantiate environment object
  double (*actions)[ITEMNUM] = [];
  int window_size = WINDOWSIZE;
  bool render_mode = false;
  double drcmax = DRCMAX;
  int input_size = INPUTSIZE;

  Env env(actions, window_size, render_mode, drcmax, input_size);
  
  //   start test loop
  int count = 0;

  int seed = -1;
  int action;

  env.reset(seed);
  action = 

  // Create a vector of inputs.
  std::vector<torch::jit::IValue> inputs;
  inputs.push_back(torch::ones({3, 9}));

  // Execute the model and turn its output into a tensor.
  at::Tensor output = module.forward(inputs).toTensor();
  std::cout << output << std::endl;
}
