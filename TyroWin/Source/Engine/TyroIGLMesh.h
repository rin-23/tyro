#include "RAEnginePrerequisites.h"
#include <Eigen/Core>
#include "RAES2DefaultTriMesh.h"

namespace tyro
{
class ES2IGLMesh : public ES2DefaultTriMesh
{
public:
    ES2IGLMesh() {}

    virtual ~ES2IGLMesh() {}
    
    static ES2IGLMeshSPtr Create(Eigen::MatrixXd& V, Eigen::MatrixXi& F, Eigen::MatrixXd N);
    
public: //Serialization

protected:
    void Init(Eigen::MatrixXd& V, Eigen::MatrixXi& F, Eigen::MatrixXd N);
private:
    
};
}