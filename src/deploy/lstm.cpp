#include <torch/script.h> // One-stop header.

#include <iostream>
#include <memory>

int main(int argc, const char* argv[]) {
  if (argc != 2) {
    std::cerr << "usage: LSTM ../model/LSTM_hidden10_layer10.pt\n";
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
  
  // Create a vector of inputs.
//  std::vector<torch::jit::IValue> input1;
//  std::vector<torch::jit::IValue> input2;
//  input1.push_back(torch::randn({10, 9}));
//  input2.push_back(torch::randn({8}));
//  torch::List<torch::Tensor> x;
//  x.push_back(input1); 
//  x.push_back(input2);
//  std::vector<torch::jit::IValue> inputs(0);
//  inputs.emplace_back(x);
  
  // Execute the model and turn its output into a tensor.
//  at::Tensor output = module.forward(inputs).toTensor();
//  std::cout << output.slice(/*dim=*/1, /*start=*/0, /*end=*/5) << '\n';
}
