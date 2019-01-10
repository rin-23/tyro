#pragma once

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
        
        void setBshapes(const std::vector<std::string>& obj_paths);

        //set weight for bshape
        void setWeight(const std::string& bname, double w);

        //set weights for all bshapes
        void setWeights(const std::vector<std::string>& bnames, std::vector<double>& weights);

        // get current face expression given weights
        void getExpression(Eigen::MatrixXd& V, Eigen::MatrixXi& F, Eigen::MatrixXd& N);

        // get blendshape names
        const std::vector<std::string>& getBnames();

    private:

        struct BShape
        {
            double weight;
            Eigen::MatrixXd V;
        };

        //int m_num_bshapes; // how many bshapes
        std::vector<std::string> mBnames;
        std::vector<BShape> mBdata;
        //std::map<std::string, BShape> m_bshapes; // map bshape name to weight value
        
        //std::vector<Eigen::MatrixXd> m_normalsN; // normal data of bshapes
        //std::vector<Eigen::MatrixXd> m_bshapesV; // vertex data of bshapes
        
        Eigen::MatrixXd mNneut; //normal data of the neuteral expression
        Eigen::MatrixXd mVneut; //vertex data of the neuteral expression
        Eigen::MatrixXi mF; // connectivity. Assume to be same for all bshapes and neut mesh
};
}