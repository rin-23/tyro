#pragma once

#include "RAEnginePrerequisites.h"
#include <Eigen/Core>
#include "RAES2TriMesh.h"
#include "RAES2Polyline.h"

namespace tyro
{
class IGLMesh : public ES2TriMesh
{
public:
    IGLMesh() {
        isPBR = false;
    }

    virtual ~IGLMesh() {}
    
    static IGLMeshSPtr Create(const Eigen::MatrixXd& V, const Eigen::MatrixXi& F, const Eigen::MatrixXd& N, const Eigen::MatrixXd& C);
    static IGLMeshSPtr Create(const Eigen::MatrixXd& V, const Eigen::MatrixXi& F, const Eigen::MatrixXd& N, const Eigen::MatrixXd& C, const Eigen::VectorXd& AO);
    static IGLMeshSPtr Create(const Eigen::MatrixXd& V, const Eigen::MatrixXi& F, const Eigen::MatrixXd& N, const Eigen::Vector3d& color);

    static IGLMeshSPtr CreateColor(const Eigen::MatrixXd& V, const Eigen::MatrixXi& F, const Eigen::MatrixXd& N, const Eigen::MatrixXd& C, const Eigen::VectorXd& error, float max_error);
    
    Eigen::MatrixXd V; //position data
    Eigen::MatrixXi F; //face data
    Eigen::MatrixXd N; //normal data
    Eigen::MatrixXd C; //color data

    virtual void UpdateUniformsWithCamera(const Camera* camera) override;

protected:
    void Init(const Eigen::MatrixXd& V, const Eigen::MatrixXi& F, const Eigen::MatrixXd& N, const Eigen::MatrixXd& C);
    void Init(const Eigen::MatrixXd& V, const Eigen::MatrixXi& F, const Eigen::MatrixXd& N, const Eigen::MatrixXd& C, const Eigen::VectorXd& AO);
    void InitColor(const Eigen::MatrixXd& V, const Eigen::MatrixXi& F, const Eigen::MatrixXd& N, const Eigen::MatrixXd& C, const Eigen::VectorXd& Error, float max_error);

private:
    bool isPBR;
    
};


class IGLMeshWireframe : public ES2Polyline
{
public:
    IGLMeshWireframe() {}

    virtual ~IGLMeshWireframe() {}
    
    //Generate wireframe from unique edges
    static IGLMeshWireframeSPtr Create(const Eigen::MatrixXd& V, const Eigen::MatrixXi& uE, const Eigen::MatrixXd& uC);
    //static IGLMeshWireframeSPtr Create(const Eigen::MatrixXd& V, const Eigen::MatrixXi& F, const Eigen::Vector3d& color);

    virtual void UpdateUniformsWithCamera(const Camera* camera) override;
    
protected:
    void Init(const Eigen::MatrixXd& V, const Eigen::MatrixXi& F, const Eigen::MatrixXd& C);

private:

};
}