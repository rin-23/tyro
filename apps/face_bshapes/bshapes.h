#pragma once

#include <string>
#include <Eigen/Dense>
#include <Eigen/Core>
#include <vector>
#include <map>
#include "RAEnginePrerequisites.h"
#include <cereal/types/vector.hpp>
#include <cereal/cereal.hpp>

#include "eigen_cerealisation.h"
#include <iostream>
#include <fstream>

#include <cereal/types/base_class.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/polymorphic.hpp>
#include <cereal/access.hpp>
#include <cereal/archives/binary.hpp>

namespace tyro
{
class FaceModel 
{
    public:
        FaceModel();
        
        ~FaceModel();

        void setNeuteralMesh(const std::string& obj_path);
        
        void setBshapes(const std::map<std::string, std::string>& bmap);
        
        void serialize(const std::string& path);
        void deserialize(const std::string& path);

        //set weight for bshape
        void setWeight(const std::string& bname, double w);

        //set weights for all bshapes
        void setWeights(const std::vector<std::string>& bnames, std::vector<double>& weights);

        // get current face expression given weights
        void getExpression(Eigen::MatrixXd& V, Eigen::MatrixXi& F, Eigen::MatrixXd& N);

        // get blendshape names
        const std::vector<std::string>& getBnames();

        // get lower data
        //void getLowerNames(std::vector<std::string>& lower_bnames);

        // TODO: get lower data
        // void getUpperWeights(std::vector<std::string>& lower_bnames, std::vector<double>& lower_weights);
        
    private:

        struct BShape
        {
            double weight;
            Eigen::MatrixXd V;

            template<class Archive>
            void save(Archive & archive) const
            {   
                archive(weight);
                archive(V);
             
            }
            
            template<class Archive>
            void load(Archive & archive)
            {   
                archive(weight);
                archive(V);          
            }
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