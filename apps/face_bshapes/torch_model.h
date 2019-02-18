
#pragma once
#include <torch/torch.h>
#include <torch/script.h> 
#include <string>
#include <Eigen/Dense>
#include <vector>
#include <map>
#include "RAEnginePrerequisites.h"


namespace tyro
{
class TorchModel 
{
    public:
        TorchModel();
        
        ~TorchModel();

        void Init(const std::string& path_to_model);

        void Compute(Eigen::MatrixXd& input, Eigen::MatrixXd& output);
        void Compute(std::vector<double>& input, std::vector<double>& output);
      
    private:

        //class torch::jit::script::Module;
        std::shared_ptr<torch::jit::script::Module> mModule;
};
}

