#include "torch_model.h"
#include <iostream>

namespace tyro
{

TorchModel::TorchModel()
:
mModule(nullptr) 
{}

TorchModel::~TorchModel() 
{}

void TorchModel::Init(const std::string& path_to_model) 
{
    // Deserialize the ScriptModule from a file using torch::jit::load().
    //std::string path_to_model("/home/rinat/Workspace/FacialManifoldSource/data_anim/traced.pth"); 
    mModule = torch::jit::load(path_to_model);
    mModule->to(at::ScalarType::Double);
    if(mModule == nullptr) 
    {
        RA_LOG_ERROR_ASSERT("Didnt initialize torch model");
    }
}

void TorchModel::Compute(Eigen::MatrixXd& input, Eigen::MatrixXd& output)
{
    int rows = input.rows();
    int cols = input.cols();

    Eigen::MatrixXd inputT = input.transpose(); // because eigen is in column-major but Tensor is in row-major
    torch::Tensor t_in = torch::from_blob(inputT.data(), {rows, cols}, torch::TensorOptions().dtype(torch::kFloat64));

    std::vector<torch::jit::IValue> inputs;
    inputs.push_back(t_in);
    auto ll = mModule->forward(inputs);
    at::Tensor t_out = ll.toTensor();
    
    typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::ColMajor>  matrix_type;
    double* data = (double*)t_out.data_ptr();
    output = Eigen::Map<matrix_type>(data, rows, cols);
}

void TorchModel::Compute(std::vector<double>& input, std::vector<double>& output)
{
    int rows = 1;
    int cols = input.size();

    torch::Tensor t_in = torch::from_blob(input.data(), {rows, cols}, torch::TensorOptions().dtype(torch::kFloat64));

    std::vector<torch::jit::IValue> inputs;
    inputs.push_back(t_in);
    auto ll = mModule->forward(inputs);
    at::Tensor t_out = ll.toTensor();
    
    //typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::ColMajor>  matrix_type;
    double* data = (double*)t_out.data_ptr();
    output.assign(data, data + cols);
    //output = Eigen::Map<matrix_type>(data, rows, cols);
}

}