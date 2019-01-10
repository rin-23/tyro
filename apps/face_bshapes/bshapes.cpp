#include "bshapes.h"
#include "load_mesh_sequence.h"
#include <assert.h>
#include <filesystem/path.h>

using namespace std;

namespace tyro
{
    FaceModel::FaceModel()
    {}
    
    FaceModel::~FaceModel()
    {}

    void FaceModel::setNeuteralMesh(const std::string& obj_path)
    {
        // load neuteral expression
        tyro::load_mesh(obj_path, mVneut, mNneut, mF);
    }
    
    void FaceModel::setBshapes(const std::vector<std::string>& obj_paths)
    {   
        std::vector<Eigen::MatrixXd> Nbshapes; // normal data of bshapes
        std::vector<Eigen::MatrixXd> Vbshapes; // vertex data of bshapes
        Eigen::MatrixXi Fbshapes; 
        tyro::load_meshes(obj_paths, Vbshapes, Nbshapes, Fbshapes);
        assert(mF.rows() == Fbshapes.rows());

        //m_num_bshapes = Vbshapes.size();
        mBnames.clear();
        mBdata.clear();

        for (int i = 0; i < obj_paths.size(); ++i) 
        {   
            auto p = filesystem::path(obj_paths[i]);
            auto name = p.basename();
            mBnames.push_back(name);
            mBdata.push_back({0.0, Vbshapes[i] - mVneut});
        }

        //m_weights[] = (m_num_bshapes); 
        //std::fill(m_weights.begin(), m_weights.end(), 0.0f);
    }

    //set weight for bshape
    void FaceModel::setWeight(const std::string& bshape_name, double w)
    {   
        auto idx = std::find(mBnames.begin(), mBnames.end(), bshape_name);
        if (idx == mBnames.end()) 
        {
            RA_LOG_ERROR_ASSERT("couldnt find the name");
        } 
        else 
        {   
            if (w < 0.0 || w > 1.0) 
            {
                RA_LOG_WARN("weight is out of bounds %s %f", bshape_name.c_str(), w);
            }

            int i = idx - mBnames.begin();
            mBdata[i].weight = w;
        }
    }

    void FaceModel::setWeights(const std::vector<std::string>& bnames, std::vector<double>& weights) 
    {
        for (int i=0; i<bnames.size(); ++i)  
        {
            const auto& bn = bnames[i];
            auto w = weights[i];
            setWeight(bn, w);
        }
    }

    void FaceModel::getExpression(Eigen::MatrixXd& V, Eigen::MatrixXi& F, Eigen::MatrixXd& N)
    {
        V = mVneut;

        for(const auto& bshape: mBdata) 
        {
            if (abs(bshape.weight) > 0.001)
            {
                V += bshape.weight * bshape.V;
            }
        }

        F = mF; // TODO inneficient copy if m_F never changes
        N = mNneut; // TODO need to interpolate as well
    }

    const std::vector<std::string>& FaceModel::getBnames() 
    {
        return mBnames;
    }

}