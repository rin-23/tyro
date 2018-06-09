#include "compute_deformation.h"
#include "RALogManager.h"
#include <igl/min_quad_with_fixed.h>
#include <igl/cotmatrix.h>
#include <igl/massmatrix.h>
#include <igl/unique.h>

bool tyro::compute_deformation( const std::vector<int>& vid_list, 
                                const std::vector<int>& fid_list, 
                                const std::vector<Eigen::MatrixXd>& v_data,
                                const Eigen::MatrixXi& F,
                                const Eigen::MatrixXd& AV, //average                       
                                std::vector<Eigen::MatrixXd>& rv_data)
{
    if (fid_list.size() == 0) 
    { 
        return false; 
    }
    
    int num_frames = v_data.size();
    rv_data.reserve(num_frames);
    
    //list of vids that can be deformed
    std::vector<int> vid_deform, vid_deform_dup; 
    for (auto fid : fid_list) 
    {
        int vid0 = F.row(fid)(0);
        int vid1 = F.row(fid)(1);
        int vid2 = F.row(fid)(2);
        
        //check that we dont include ones that are part of the seam 
        auto it2 = std::find(vid_list.begin(), vid_list.end(), vid0);
        if (it2 == vid_list.end()) vid_deform_dup.push_back(vid0);
        
        it2 = std::find(vid_list.begin(), vid_list.end(), vid1);
        if (it2 == vid_list.end()) vid_deform_dup.push_back(vid1);

        it2 = std::find(vid_list.begin(), vid_list.end(), vid2);
        if (it2 == vid_list.end()) vid_deform_dup.push_back(vid2);            
    }
    igl::unique(vid_deform_dup, vid_deform); //remove duplicates
    
    //list of vids that can not be deformed minus seam vids
    std::vector<int> vid_not_deform; 
    for (int vid = 0; vid < v_data[0].rows(); ++vid) 
    {
        auto it = std::find(vid_deform.begin(), vid_deform.end(), vid); 
        auto it2 = std::find(vid_list.begin(), vid_list.end(), vid);

        if (it == vid_deform.end() && it2 == vid_list.end())
        {   
            vid_not_deform.push_back(vid);
        }    
    }

    assert(vid_deform.size() + vid_not_deform.size() + vid_list.size() == v_data[0].rows());

    //Eigen::MatrixXi F = m_frame_data.f_data;
    Eigen::VectorXi b;
    int num_fixed = vid_not_deform.size() + vid_list.size();
    int num_not_fixed = vid_deform.size();
    b.resize(num_fixed);
    
    int b_index = 0;            
    for (int vid : vid_list) b(b_index++) = vid;
    for (int vid : vid_not_deform) b(b_index++) = vid;
                
    //do biharmonic stuff here.
    igl::min_quad_with_fixed_data<double> data;
    Eigen::SparseMatrix<double> C;            
    igl::cotmatrix(AV, F, C); //try average

    Eigen::SparseMatrix<double> M;	
    igl::massmatrix(AV, F, igl::MASSMATRIX_TYPE_DEFAULT, M);

    //Computer M inverse
    Eigen::SimplicialLDLT <Eigen::SparseMatrix<double>> solver;
    solver.compute(M);
    Eigen::SparseMatrix<double> I(M.rows(), M.cols());
    I.setIdentity();
    Eigen::SparseMatrix<double> M_inv = solver.solve(I);
    Eigen::SparseMatrix<double> Q = C.transpose() * M_inv * C;
    Eigen::SparseMatrix<double> Aeq;
    Q = 2 * Q;
    bool result = igl::min_quad_with_fixed_precompute(Q, b, Aeq, false, data);
    assert(result);

    for (int i =0; i < num_frames; ++i) 
    {   
        RA_LOG_INFO("Compute deformation for mesh %i out of %i", i, num_frames)
        Eigen::MatrixXd V = v_data[i];

        Eigen::MatrixXd D;
        Eigen::MatrixXd bc;
        bc.resize(num_fixed, 3);
        bc.setZero();
        b_index = 0;
        for (int vid : vid_list)
        {
            bc.row(b_index++) = AV.row(vid) - v_data[i].row(vid);
        }

        Eigen::MatrixXd B = Eigen::MatrixXd::Zero(data.n, bc.cols());
        Eigen::VectorXd Beq;
        
        result = igl::min_quad_with_fixed_solve(data, B, bc, Beq, D);
        assert(result);

        Eigen::MatrixXd V_prime = V + D;
        rv_data.push_back(V_prime);
    }   

    return true;
}

bool tyro::compute_deformation2(const std::vector<int>& vid_list, 
                                const std::vector<Eigen::MatrixXd>& v_data,
                                const Eigen::MatrixXi& F,
                                const Eigen::MatrixXd& AV, //average                       
                                std::vector<Eigen::MatrixXd>& rv_data, 
                                std::vector<Eigen::MatrixXd>& Laplacian)
{
    
    int num_frames = v_data.size();
    rv_data.reserve(num_frames);
   
    Eigen::VectorXi b;
    int num_fixed = vid_list.size();
    b.resize(num_fixed);
    
    int b_index = 0;            
    for (int vid : vid_list) 
        b(b_index++) = vid;
                 
    //do biharmonic stuff here.
    igl::min_quad_with_fixed_data<double> data;
    Eigen::SparseMatrix<double> C;            
    igl::cotmatrix(AV, F, C); //try average

    Eigen::SparseMatrix<double> M;	
    igl::massmatrix(AV, F, igl::MASSMATRIX_TYPE_DEFAULT, M);

    //Computer M inverse
    Eigen::SimplicialLDLT <Eigen::SparseMatrix<double>> solver;
    solver.compute(M);
    Eigen::SparseMatrix<double> I(M.rows(), M.cols());
    I.setIdentity();
    Eigen::SparseMatrix<double> M_inv = solver.solve(I);
    Eigen::SparseMatrix<double> Q = C.transpose() * M_inv * C;
    Eigen::SparseMatrix<double> Aeq;
    Q = 2 * Q;
    bool result = igl::min_quad_with_fixed_precompute(Q, b, Aeq, false, data);
    assert(result);

    Laplacian.resize(num_frames);
    
    Eigen::SparseMatrix<double> L = M_inv*C;
    for (int i = 0; i < num_frames; ++i) 
    {   
        RA_LOG_INFO("Compute deformation for mesh %i out of %i", i, num_frames)
        Eigen::MatrixXd V = v_data[i];

        Eigen::MatrixXd D;
        Eigen::MatrixXd bc;
        bc.resize(num_fixed, 3);
        bc.setZero();
        b_index = 0;

        for (int vid : vid_list)
        {
            bc.row(b_index++) = AV.row(vid) - v_data[i].row(vid);
        }
        //for (int vid : vid_not_deform)
        //{
        //   bc.row(b_index++) = V.row(vid);
        //}
        Eigen::MatrixXd B = Eigen::MatrixXd::Zero(data.n, bc.cols());
        Eigen::VectorXd Beq;
        
        result = igl::min_quad_with_fixed_solve(data, B, bc, Beq, D);
        assert(result);

        Eigen::MatrixXd V_prime = V + D;
        rv_data.push_back(V_prime);

        Eigen::MatrixXd LD = L*D;
        Laplacian[i] = LD;        
    }   

    return true;
}


/*
void tyro::compute_deformation(const std::vector<int>& vid_list, 
                               const std::vector<Eigen::MatrixXd>& v_data,
                               const Eigen::MatrixXd& F,
                               const Eigen::MatrixXd& AV, //average                       
                               std::vector<Eigen::MatrixXd>& rv_data,
                               Eigen::MatrixXd& rF
                              )
{
        assert(vid_list.size() > 0);
        rv_data.reserve(v_data.size());
        rF = F;

        int num_frames = v_data.size();
        for (int i =0; i < num_frames; ++i) 
        {   
            RA_LOG_INFO("Compute deformation for mesh %i out of %i", i, num_frames)
            Eigen::MatrixXd V = v_data[i];
            //Eigen::MatrixXi F = f_data;
            Eigen::VectorXi b;
            b.resize(vid_list.size());
            int b_index = 0;
            
            for (int vid : vid_list)
            {
                b(b_index++) = vid;
                //RA_LOG_INFO("vide %i", vid)
            }
           
            //do biharmonic stuff here.
            igl::min_quad_with_fixed_data<double> data;
            Eigen::SparseMatrix<double> L;
            
            igl::cotmatrix(V, F, L);

            Eigen::SparseMatrix<double> M;	
            igl::massmatrix(V, F, igl::MASSMATRIX_TYPE_DEFAULT, M);

            //Computer M inverse
            Eigen::SimplicialLDLT <Eigen::SparseMatrix<double> > solver;
            solver.compute(M);
            Eigen::SparseMatrix<double> I(M.rows(), M.cols());
            I.setIdentity();
            Eigen::SparseMatrix<double> M_inv = solver.solve(I);
            Eigen::SparseMatrix<double> Q = L.transpose() * M_inv * L;
            Eigen::SparseMatrix<double> Aeq;
            Q = 2 * Q;
            bool result = igl::min_quad_with_fixed_precompute(Q, b, Aeq, false, data);
            assert(result);

            Eigen::MatrixXd D;
            Eigen::MatrixXd bc;
            bc.resize(vid_list.size(), 3);
            b_index = 0;
            for (int vid : vid_list)
            {
                bc.row(b_index++) = AV.row(vid) - v_data[i].row(vid);
            }

            Eigen::MatrixXd B = Eigen::MatrixXd::Zero(data.n, bc.cols());
            Eigen::VectorXd Beq;
            //Beq.resize(bc.)
            result = igl::min_quad_with_fixed_solve(data, B, bc, Beq, D);
            assert(result);

            Eigen::MatrixXd V_prime = V + D;
            rv_data.push_back(V_prime);
            
        }
    }
*/