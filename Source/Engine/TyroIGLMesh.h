#pragma once

#include "RAEnginePrerequisites.h"
#include <Eigen/Core>
#include "RAES2DefaultTriMesh.h"
#include "RAES2DefaultPolyline.h"

namespace tyro
{
class IGLMesh : public ES2DefaultTriMesh
{
public:
    IGLMesh() {}

    virtual ~IGLMesh() {}
    
    static IGLMeshSPtr Create(Eigen::MatrixXd& V, Eigen::MatrixXi& F, Eigen::MatrixXd N);
    
    Eigen::MatrixXd V;
    Eigen::MatrixXi F;
    Eigen::MatrixXd N;

protected:
    void Init(Eigen::MatrixXd& V, Eigen::MatrixXi& F, Eigen::MatrixXd N);
private:
    
};


class IGLMeshWireframe : public ES2DefaultPolyline
{
public:
    IGLMeshWireframe() {}

    virtual ~IGLMeshWireframe() {}
    
    static IGLMeshWireframeSPtr Create(Eigen::MatrixXd& V, Eigen::MatrixXi& F);
    
protected:
    void Init(Eigen::MatrixXd& V, Eigen::MatrixXi& F);
private:

};
}