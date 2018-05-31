//#pragma once
#include <igl/min_quad_with_fixed.h>
#include <Eigen/Core>
#include <Eigen/Sparse>
#include <igl/cotmatrix.h>
#include <igl/massmatrix.h>
#include <iostream>
#include <igl/hessian_energy.h> 
#include <igl/read_triangle_mesh.h>
#include <igl/edges.h>
#include <igl/components.h>
#include <igl/remove_unreferenced.h>
#include <Eigen/Core>
#include <Eigen/SparseCholesky>
#include <igl/edge_lengths.h>
#include <igl/adjacency_list.h>

// Given a mesh (`V`,`F`) and data specified per-vertex (`G`), smooth this data
// using a single implicit Laplacian smoothing step.
//
// Inputs:
//   V  #V by 3 list of mesh vertex positions
//   F  #F by 3 list of mesh triangle indices into V
//   G  #V by dim list of per-vertex data: could be scalar- (`dim =1`) or
//     vector-valued (`dim>1`).
//   lambda  >0 smoothing parameter also known as the "time step" `dt`
// Outputs: 
//   U  #V by dim list of smoothed data
//

typedef Eigen::SparseMatrix<double> SparseMat;
namespace tyro {
void smooth(
    const Eigen::MatrixXd & V,
    const Eigen::MatrixXi & F,
    const Eigen::VectorXd & znoisy,
    double lambda,
    Eigen::VectorXd & zsmooth)
{
    SparseMat L, M;
    igl::cotmatrix(V, F, L);
    igl::massmatrix(V, F, igl::MASSMATRIX_TYPE_BARYCENTRIC, M);
    
    Eigen::SimplicialLDLT<SparseMat> solver(M);
    SparseMat MinvL = solver.solve(L);
    SparseMat QL = L.transpose()*MinvL;
    SparseMat QH;
    igl::hessian_energy(V, F, QH);

    const double al = 8e-4;
    Eigen::SimplicialLDLT<SparseMat> lapSolver(al*QL + (1.-al)*M);
    Eigen::VectorXd zl = lapSolver.solve(al*M*znoisy);
    const double ah = 5e-6;
    Eigen::SimplicialLDLT<SparseMat> hessSolver(ah*QH + (1.-ah)*M);
    Eigen::VectorXd zh = hessSolver.solve(ah*M*znoisy);   
    zsmooth = zh;

}
void smooth2(
    const Eigen::MatrixXd & inV,
    const Eigen::MatrixXi & F,
    const Eigen::MatrixXd & znoisy,
    double lambda,
    Eigen::MatrixXd & zsmooth)
{
    SparseMat L, M;
    Eigen::MatrixXd V  = inV;///inV.minCoeff();
    igl::cotmatrix(V, F, L);
    igl::massmatrix(V, F, igl::MASSMATRIX_TYPE_BARYCENTRIC, M);
    
    Eigen::MatrixXd rhs = M*znoisy;
    
    Eigen::SparseMatrix<double> A = -lambda*L;
   
    for(int i = 0; i < A.rows(); i++){
        A.coeffRef(i,i) += M.diagonal()[i];
    }
    
    Eigen::SimplicialLDLT<Eigen::SparseMatrix<double> > solver;
    solver.compute(A);
    zsmooth = solver.solve(rhs);

}

void smooth3(
    const Eigen::MatrixXd & inV,
    const Eigen::MatrixXi & F,
    const Eigen::MatrixXd & znoisy,
     double lambda,
    Eigen::MatrixXd & zsmooth)
{
    
 //   VectorXd EL; //double area

    //igl::edge_lengths(inV, F, EL);

    std::vector<std::vector<int>> A;
    igl::adjacency_list(F, A);
    zsmooth.resize(znoisy.rows(), 1);
    for (int vid = 0; vid < inV.rows(); ++vid)
    {
        auto& N = A[vid];
        double total = 0;
        for (auto& vid_n : N) 
        {
            double value = znoisy(vid_n, 0);
            total += value;
        }
        total /= N.size();
        zsmooth(vid, 0) = total;
    }
 }

 void smooth4(const Eigen::MatrixXd& V,
                       const Eigen::MatrixXi& F,
                       Eigen::MatrixXd& S,
                       double lambda,
                       Eigen::MatrixXd& zsmooth) 
{
  /*
  % LAPLACIAN_SMOOTH smooth a mesh using implicit/explicit laplacian smoothing
  %
  % [U] = laplacian_smooth(V,F)
  % [U] = laplacian_smooth(V,F,L_method,b,lambda,method,S)
  % [U,Uall] = laplacian_smooth(V,F,L_method,b,lambda,method,S,max_iter)
  % 
  % Inputs:
  %   V  #V x 3 matrix of vertex coordinates
  %   F  #F x 3  matrix of indices of triangle corners
  %   L_method  method for laplacian
  %      'uniform'
  %      'cotan'
  %   b  list of indices of fixed vertices
  %   lambda  diffusion speed parameter {0.1}
  %   method  method to use:
  %     'implicit' (default)
  %     'explicit'
  %   S  scalar fields to smooth (default V)
  % Outputs:
  %   U  #V x 3 list of new vertex positions
  %   Uall  #V x 3 x iters list of new vertex positions for each iteration
  %   
*/
using namespace Eigen;

 // % number of vertices
    int n = V.rows();
 // % nuymber of dimensions
    int dim = V.cols();
  
    SparseMat I = SparseMat(n,n);
    I.setIdentity();
 
    double tol = 0.001;
 
    SparseMat L, M;
    igl::cotmatrix(V,F,L);
    igl::massmatrix(V, F, igl::MASSMATRIX_TYPE_BARYCENTRIC, M);

    Eigen::SimplicialLDLT<SparseMat> solver(M);
    SparseMat MinvL = solver.solve(L);
    
    SparseMat Q = (I-lambda*MinvL);
    VectorXi known;
    MatrixXd Y;
    VectorXd Beq;
    SparseMat Aeq;
    MatrixXd Z;
   
    igl::min_quad_with_fixed(Q,
                            -S,
                            known,
                            Y,
                            Aeq,
                            Beq,
                            false,
                            Z);
    
        //igl::min_quad_with_fixed_data<double> data;
        //bool result = igl::min_quad_with_fixed_precompute(Q, known, Aeq, false, data);
        //assert(result);

        /*
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
        // {
        //    bc.row(b_index++) = V.row(vid);
        // }

        Eigen::MatrixXd B = Eigen::MatrixXd::Zero(data.n, bc.cols());
        Eigen::VectorXd Beq;
        
        result = igl::min_quad_with_fixed_solve(data, B, bc, Beq, D);
        */
    }

}
/*
IGL_INLINE bool min_quad_with_fixed(
    const Eigen::SparseMatrix<T>& A,
    const Eigen::PlainObjectBase<DerivedB> & B,
    const Eigen::PlainObjectBase<Derivedknown> & known,
    const Eigen::PlainObjectBase<DerivedY> & Y,
    const Eigen::SparseMatrix<T>& Aeq,
    const Eigen::PlainObjectBase<DerivedBeq> & Beq,
    const bool pd,
    Eigen::PlainObjectBase<DerivedZ> & Z);

*/
