#include "RAEnginePrerequisites.h"
#include <Eigen/Core>
#include "RAES2DefaultTriMesh.h"

namespace tyro
{
class IGLMesh : public ES2DefaultTriMesh
{
public:
    IGLMesh() {}

    virtual ~IGLMesh() {}
    
    static IGLMeshSPtr Create(Eigen::MatrixXd& V, Eigen::MatrixXi& F, Eigen::MatrixXd N);
    
public: //Serialization

protected:
    void Init(Eigen::MatrixXd& V, Eigen::MatrixXi& F, Eigen::MatrixXd N);
private:
    
};
}