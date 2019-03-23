#pragma once

#include "RAEnginePrerequisites.h"
#include <Eigen/Core>
#include "RAES2TriMesh.h"
#include "RAES2Polyline.h"

namespace tyro
{
class IGLMeshTwo : public ES2TriMesh
{
public:
    IGLMeshTwo() {}
    virtual ~IGLMeshTwo() {}

    static IGLMeshTwoSPtr Create(const Eigen::MatrixXd& V, const Eigen::MatrixXi& F, const Eigen::MatrixXd& N, const Eigen::Vector3d& color); 
    
    void UpdateData(const Eigen::MatrixXd& V, const Eigen::MatrixXi& F, const Eigen::MatrixXd& N, const Eigen::MatrixXd& C);
    
    void Init(const Eigen::MatrixXd& V, const Eigen::MatrixXi& F, const Eigen::MatrixXd& N, const Eigen::MatrixXd& C);

    virtual void UpdateUniformsWithCamera(const Camera* camera) override;
};
}