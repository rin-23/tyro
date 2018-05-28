#include <igl/read_triangle_mesh.h>
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <vector>

template <
  typename DerivedV,
  typename DerivedF,
  typename DerivedS,
  typename Derivedval,
  typename DerivedU,
  typename DerivedG,
  typename DerivedJ,
  typename BCtype,
  typename DerivedSU,
  typename DerivedL>
  IGL_INLINE void slice_isolines(
    const Eigen::MatrixBase<DerivedV> & V,
    const Eigen::MatrixBase<DerivedF> & F,
    const Eigen::MatrixBase<DerivedS> & S,
    const Eigen::MatrixBase<Derivedval> & val,
    Eigen::PlainObjectBase<DerivedU> & U,
    Eigen::PlainObjectBase<DerivedG> & G,
    Eigen::PlainObjectBase<DerivedJ> & J,
    Eigen::SparseMatrix<BCtype> & BC,
    Eigen::PlainObjectBase<DerivedSU> & SU,
    Eigen::PlainObjectBase<DerivedL> & L)
{
  // Lazy implementation using vectors
  assert(val.size() == 1);
  const int isoval_i = 0;
  auto isoval = val(isoval_i);
  std::vector<std::vector<typename DerivedG::Scalar> > vG;
  std::vector<typename DerivedJ::Scalar> vJ;
  std::vector<typename DerivedL::Scalar> vL;
  std::vector<Eigen::Triplet<BCtype> > vBC;
  int Ucount = 0;
  for(int i = 0;i<V.rows();i++)
  {
    vBC.emplace_back(Ucount,i,1.0);
    Ucount++;
  }
  // Loop over each face
  for(int f = 0;f<F.rows();f++)
  {
    bool Psign[2];
    int P[2];
    int count = 0;
    for(int p = 0;p<3;p++)
    {
      const bool psign = S(F(f,p)) > isoval;
      // Find crossings
      const int n = (p+1)%3;
      const bool nsign = S(F(f,n)) > isoval;
      if(psign != nsign)
      {
        P[count] = p;
        Psign[count] = psign;
        // record crossing
        count++;
      }
    }
    assert(count == 0  || count == 2);
    switch(count)
    {
      case 0:
      {
        // Easy case
        std::vector<typename DerivedG::Scalar> row = {F(f,0),F(f,1),F(f,2)};
        vG.push_back(row);
        vJ.push_back(f);
        vL.push_back( S(F(f,0))>isoval ? isoval_i+1 : isoval_i );
        break;
      }
      case 2:
      {
        // Cut case
        // flip so that P[1] is the one-off vertex
        if(P[0] == 0 && P[1] == 2)
        {
          std::swap(P[0],P[1]);
          std::swap(Psign[0],Psign[1]);
        }
        assert(Psign[0] != Psign[1]);
        // Create two new vertices
        for(int i = 0;i<2;i++)
        {
          const double bci = (isoval - S(F(f,(P[i]+1)%3)))/
            (S(F(f,P[i]))-S(F(f,(P[i]+1)%3)));
          vBC.emplace_back(Ucount,F(f,P[i]),bci);
          vBC.emplace_back(Ucount,F(f,(P[i]+1)%3),1.0-bci);
          Ucount++;
        }
        const int v0 = F(f,P[0]);
        const int v01 = Ucount-2;
        assert(((P[0]+1)%3) == P[1]);
        const int v1 = F(f,P[1]);
        const int v12 = Ucount-1;
        const int v2 = F(f,(P[1]+1)%3);
        // v0
        // |  \
        // |   \
        // |    \
        // v01   \
        // |      \
        // |       \
        // |        \
        // v1--v12---v2
        typedef std::vector<typename DerivedG::Scalar> Row;
        {Row row = {v01,v1,v12}; vG.push_back(row);vJ.push_back(f);vL.push_back(Psign[0]?isoval_i:isoval_i+1);}
        {Row row = {v12,v2,v01}; vG.push_back(row);vJ.push_back(f);vL.push_back(Psign[1]?isoval_i:isoval_i+1);}
        {Row row = {v2,v0,v01}; vG.push_back(row) ;vJ.push_back(f);vL.push_back(Psign[1]?isoval_i:isoval_i+1);}
        break;
      }
      default: assert(false);
    }
  }
  igl::list_to_matrix(vG,G);
  igl::list_to_matrix(vJ,J);
  igl::list_to_matrix(vL,L);
  BC.resize(Ucount,V.rows());
  BC.setFromTriplets(vBC.begin(),vBC.end());
  U = BC * V;
  SU = BC * S;
}

/*
int main(int argc, char *argv[])
{
  Eigen::MatrixXd V;
  Eigen::MatrixXi F;
  igl::read_triangle_mesh(argv[1],V,F);
  Eigen::VectorXd S = V.col(1);
  S.array() -= S.minCoeff();
  S /= S.maxCoeff();
  Eigen::VectorXd val(1);
  val(0) = 0.5;
  
  Eigen::MatrixXd U;
  Eigen::MatrixXi G;
  Eigen::VectorXi J;
  Eigen::SparseMatrix<double> BC;
  Eigen::VectorXd SU,L;
  slice_isolines(V,F,S,val, U,G,J,BC,SU,L);
  //igl::opengl::glfw::Viewer v;
  //v.data().set_mesh(U,G);
  //v.data().set_colors(L);
  //v.launch();
}
*/


/*
% L is a #F by 1 array of per-face labels e.g., 0 or 1
% Significantly faster than building the transpose

W = repmat(doublearea(Vavg,F),3,1);
W = sparse(repmat(1:size(F,1), 1, 3), F(:), W, size(F,1), size(Vavg,1));
W = W./sum(W,1);
W = W';
LV = W*L;

% LV is a #V by 1 array of per-vertex labels
% Smooth function over the surface. Increasing 10 leads to more smoothness.

LV = laplacian_smooth(Vavg,F,'cotan',[],10,'implicit',LV,1);

% Cut the mesh along the LV=0.5 isoline, this creates smaller triangles along
% the isolines so that the segmentation perfectly respects the smooth boundary.

[U,G,J,BC,LU,LG] = slice_isolines(Vavg,F,LV,0.5,'Manifold',true);

% U  #U by 3 are vertices of a new triangle mesh
% G  #G by 3 are the face indices of this mesh, indexing rows of U
% floor

LG = LG-1;

% LG   #G by 1 array of per-face labels
*/