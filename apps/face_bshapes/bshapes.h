
#include <string>
#include <Eigen/Dense>
#include <vector>
#include <map>
#include "RAEnginePrerequisites.h"


namespace tyro
{
class FaceModel 
{
    public:
        FaceModel();
        
        ~FaceModel();

        void setNeuteralMesh(const std::string& obj_path);
        
        void setBshapes(std::vector<std::string>& obj_paths);

        //set weight for bshape
        void setWeight(const std::string& bshape_name, double w);

        // get current face expression given weights
        void getExpression(Eigen::MatrixXd& V, Eigen::MatrixXi& F, Eigen::MatrixXd& N);

    private:
        struct BShape
        {
            std::string name;
            double weight;
            Eigen::MatrixXd V;
        };

        //int m_num_bshapes; // how many bshapes
        std::map<std::string, BShape> m_bshapes; // map bshape name to weight value
        
        //std::vector<Eigen::MatrixXd> m_normalsN; // normal data of bshapes
        //std::vector<Eigen::MatrixXd> m_bshapesV; // vertex data of bshapes
        
        Eigen::MatrixXd m_Nneut; //normal data of the neuteral expression
        Eigen::MatrixXd m_Vneut; //vertex data of the neuteral expression
        Eigen::MatrixXi m_F; // connectivity. Assume to be same for all bshapes and neut mesh
};
}