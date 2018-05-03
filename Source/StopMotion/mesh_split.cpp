#include "mesh_split.h"
//#include <igl/oriented_facets.h>
#include <igl/edge_flaps.h>
#include <set>
#include <queue>

using namespace Eigen;

void tyro::mesh_split(const Eigen::MatrixXi& F, 
                      const Eigen::VectorXi& E_seam,
                      Eigen::MatrixXi& F1, 
                      Eigen::MatrixXi& F2) 
{   
    //MatrixXi E;
    //igl::oriented_facets(F, E);

    //   E  #E by 2 list of edge indices into V.
    //   EMAP #F*3 list of indices into E, mapping each directed edge to unique
    //     unique edge in E
    //   EF  #E by 2 list of edge flaps, EF(e,0)=f means e=(i-->j) is the edge of
    //     F(f,:) opposite the vth corner, where EI(e,0)=v. Similarly EF(e,1) "
    //     e=(j->i)
    //   EI  #E by 2 list of edge flap corners (see above).

    VectorXi fEMAP;
    MatrixXi fE, fEF, fEI;
    igl::edge_flaps(F, fE, fEMAP, fEF, fEI);

    using EdgeSet = std::set<int>; 
    EdgeSet edge_set[2];

    using FaceSet = std::set<int>;
    FaceSet face_set[2];
    
    using EdgeQueue = std::queue<int>;
    EdgeQueue edge_queue[2];

    //initialize
    for (int e = 0; e < E_seam.size(); ++e) 
    {   
        int ue_idx = fEMAP(E_seam(e)); //find unique edge idx into E
        
        for (int j =0; j < 2; ++j) 
        {
            int f = fEF(ue_idx, j);
            face_set[j].insert(f);
            
            for(int v = 0; v < 3; ++v) // loop over edges
            {
                // get edge id
                const int e = fEMAP(v*F.rows()+f);
                if (e != ue_idx) 
                {
                    edge_queue[j].push(e);
                } 
                else 
                {
                    edge_set[j].insert(e);
                }
            } 
        }
    }

    auto glambda = [](const MatrixXi& F,
                      const VectorXi& fEMAP, 
                      const MatrixXi& fEF, 
                      EdgeQueue& edge_queue,
                      EdgeSet& edge_set, 
                      FaceSet& face_set) 
    { 
        while (!edge_queue.empty()) 
        {
            int ue_idx = edge_queue.back();
            edge_queue.pop();
            edge_set.insert(ue_idx);

            //check both sides of the edges
            for (int j = 0 ; j < 2; ++j) 
            {
                int f = fEF(ue_idx, j);
                if (face_set.count(f) != 0) //already processed all edges 
                {
                    continue;
                }

                //loop over edges
                for(int v = 0; v < 3; ++v)
                {                
                    const int e = fEMAP(v*F.rows()+f); // get edge id
                    if (edge_set.count(e) == 0) 
                    { 
                        edge_queue.push(e);
                    }
                }
                face_set.insert(f);
            }
        }
        printf("stop here");
    };

    //flood fill
    glambda(F, fEMAP, fEF, edge_queue[0], edge_set[0], face_set[0]); //one side
    //glambda(F, fEMAP, fEF, edge_queue[1], edge_set[1], face_set[1]); //another side

    F1.resize(face_set[0].size(), 3);

    int rowid = 0;
    for (auto& fid : face_set[0]) 
    {
        F1.row(rowid++) = F.row(fid);
    }
}