#include<iostream>

// TODO 3: include dependencies
#include "test.h"

// pytorch C++ API https://pytorch.org/cppdocs/
#include <torch/script.h> // One-stop header.

// define environment settings
#define WINDOWSIZE  2
#define INPUTSIZE   9
#define DRCMAX      1e6


int main(int argc, const char* argv[]) {
  if (argc != 2) {
    std::cerr << "usage: DQN <path to model>\n";
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

  // load env
  helloWorld();

  // Create a vector of inputs.
  std::vector<torch::jit::IValue> inputs;
  inputs.push_back(torch::ones({3, 9}));

  // Execute the model and turn its output into a tensor.
  at::Tensor output = module.forward(inputs).toTensor();
  std::cout << output << std::endl;
}
