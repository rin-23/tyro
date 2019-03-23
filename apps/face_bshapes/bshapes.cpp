#include "bshapes.h"
#include "load_mesh_sequence.h"
#include <assert.h>
#include "common.h"

using namespace std;

namespace tyro
{
    FaceModel::FaceModel()
    {}
    
    FaceModel::~FaceModel()
    {}

    void FaceModel::InitDefault() 
    {
        setNeuteralMesh(NEUT);
        setBshapes(BSHAPES_MAP); // ORDER IS VERY IMPORTANT to match BSHAPES in arig.py
        setEyes(EYES);
    }

    void FaceModel::setNeuteralMesh(const std::string& obj_path)
    {
        // load neuteral expression
        tyro::load_mesh(obj_path, mVneut, mNneut, mF);
    }
    
    void FaceModel::setBshapes(const std::map<std::string, std::string>& bmap)
    {   
        Eigen::MatrixXi Fbshapes; 
        mBnames.clear();
        mBdata.clear();

        for (const auto& kv : bmap) 
        {
            auto maya_au = kv.first;
            auto obj_path = kv.second;
            Eigen::MatrixXd V, N;
            tyro::load_mesh(obj_path, V, N, Fbshapes);
            assert(mF.rows() == Fbshapes.rows());

            mBnames.push_back(maya_au);
            mBdata.push_back({0.0, V - mVneut});
        }
    }

    void FaceModel::setEyes(const std::vector<std::string>& eyes) 
    {
        tyro::load_mesh(eyes[0], mRightEye.whiteV, mRightEye.whiteN, mRightEye.whiteF);
        tyro::load_mesh(eyes[1], mRightEye.brownV, mRightEye.brownN, mRightEye.brownF);
        tyro::load_mesh(eyes[2], mRightEye.blackV, mRightEye.blackN, mRightEye.blackF);

        tyro::load_mesh(eyes[3], mLeftEye.whiteV, mLeftEye.whiteN, mLeftEye.whiteF);
        tyro::load_mesh(eyes[4], mLeftEye.brownV, mLeftEye.brownN, mLeftEye.brownF);
        tyro::load_mesh(eyes[5], mLeftEye.blackV, mLeftEye.blackN, mLeftEye.blackF);
    }

    void FaceModel::serialize(const std::string& path) 
    {
        std::ofstream os(path, std::ios::binary);
        cereal::BinaryOutputArchive archive(os);
        archive(mBnames);
        archive(mBdata);
        archive(mNneut);
        archive(mVneut);
        archive(mF);
        archive(mLeftEye);
        archive(mRightEye);
    }

    void FaceModel::deserialize(const std::string& path) 
    {
        std::ifstream in(path, std::ios::binary);
        cereal::BinaryInputArchive archive(in);
        archive(mBnames);
        archive(mBdata);
        archive(mNneut);
        archive(mVneut);
        archive(mF);
        archive(mLeftEye);
        archive(mRightEye);
    }

    //set weight for bshape
    void FaceModel::setWeight(const std::string& bshape_name, double w)
    {   
        auto idx = std::find(mBnames.begin(), mBnames.end(), bshape_name);
        if (idx == mBnames.end()) 
        {
            RA_LOG_WARN("couldnt find the name %s", bshape_name.c_str());
        } 
        else 
        {   
            if (w < -1.0 || w > 1.0) 
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