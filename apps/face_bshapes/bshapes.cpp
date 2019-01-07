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
        tyro::load_mesh(obj_path, m_Vneut, m_Nneut, m_F);
    }
    
    void FaceModel::setBshapes(std::vector<std::string>& obj_paths)
    {   
        std::vector<Eigen::MatrixXd> Nbshapes; // normal data of bshapes
        std::vector<Eigen::MatrixXd> Vbshapes; // vertex data of bshapes
        Eigen::MatrixXi Fbshapes; 
        tyro::load_meshes(obj_paths, Vbshapes, Nbshapes, Fbshapes);
        assert(m_F.rows() == Fbshapes.rows());

        //m_num_bshapes = Vbshapes.size();

        for (int i = 0; i < obj_paths.size(); ++i) 
        {   
            auto p = filesystem::path(obj_paths[i]);
            auto name = p.basename();
            m_bshapes[name] = {name, 0.2, Vbshapes[i] - m_Vneut}; // TODO expensive copy operation
        }

        //m_weights[] = (m_num_bshapes); 
        //std::fill(m_weights.begin(), m_weights.end(), 0.0f);
    }

    //set weight for bshape
    void FaceModel::setWeight(const std::string& bshape_name, double w)
    {
        if ( m_bshapes.find(bshape_name) == m_bshapes.end() ) 
        {
            RA_LOG_ERROR_ASSERT("couldnt find the name");
        } 
        else 
        {
            if (w < 0.0 || w > 1.0) 
            {
                RA_LOG_WARN("weight is out of bounds %f", w);
            }

            m_bshapes[bshape_name].weight = w;
        }
    }

    // get current face expression given weights
    void FaceModel::getExpression(Eigen::MatrixXd& V, Eigen::MatrixXi& F, Eigen::MatrixXd& N)
    {
        V = m_Vneut;

        for(auto& imap: m_bshapes) 
        {
            BShape& bshape = imap.second;
            if (bshape.weight > 0.0)
            {
                V += bshape.weight * bshape.V;
            }
        }

        F = m_F; // TODO inneficient copy if m_F never changes
        N = m_Nneut; // TODO need to interpolate as well
    }
}