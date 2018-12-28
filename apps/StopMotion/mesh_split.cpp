#include "mesh_split.h"
//#include <igl/oriented_facets.h>
#include <igl/edge_flaps.h>
#include <set>
#include <queue>

using namespace Eigen;

void tyro::mesh_split(const Eigen::MatrixXi& F, 
                      const Eigen::VectorXi& uEI, // indicies of seam edges into unique edges matrix 
                      const Eigen::VectorXi& DMAP, // checks which directions where switched HACKY
                      Eigen::VectorXi& F1idx, //indicies of faces into F that belong to part1 
                      Eigen::VectorXi& F2idx)  // indicies of face into F that belong to part2
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
    using FaceSet = std::set<int>;
    using EdgeQueue = std::queue<int>;
    
    EdgeSet edge_set[2];
    FaceSet face_set[2];
    EdgeQueue edge_queue[2];

    //initialize face sets
    for (int e = 0; e < uEI.size(); ++e) 
    {   
        int ue_idx = uEI(e); //find unique edge idx into E
        edge_set[0].insert(ue_idx);
        edge_set[1].insert(ue_idx);

        int f1 = fEF(ue_idx, 0);
        int f2 = fEF(ue_idx, 1);
        
        if (DMAP(e) == 0) 
        {
            face_set[0].insert(f1);
            face_set[1].insert(f2);
        }
        else 
        {
            face_set[1].insert(f1);
            face_set[0].insert(f2);
        }
    }

    for (int side_idx = 0; side_idx < 2; ++side_idx)
    {   
        for (auto& f : face_set[side_idx]) 
        {   
            for(int v = 0; v < 3; ++v) // loop over edges
            {
                // get edge id
                const int e = fEMAP(v*F.rows()+f);
                if (edge_set[side_idx].count(e) == 0) 
                {
                    edge_queue[side_idx].push(e);
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
            int ue_idx = edge_queue.front();
            edge_queue.pop();
            edge_set.insert(ue_idx);

            //check both sides of the edges
            for (int j = 0 ; j < 2; ++j) 
            {
                int f = fEF(ue_idx, j);
                if (f == -1 || face_set.count(f) != 0) //already processed all edges 
                {
                    continue;
                }

                //loop over edges
                for(int v = 0; v < 3; ++v)
                {                
                    int e = fEMAP(v*F.rows()+f); // get edge id
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
    glambda(F, fEMAP, fEF, edge_queue[1], edge_set[1], face_set[1]); //another side

    
    F1idx.resize(face_set[0].size());
    int rowid = 0;
    for (auto& fid : face_set[0]) 
    {
        F1idx(rowid++) = fid;
    }

    F2idx.resize(face_set[1].size());
    rowid = 0;
    for (auto& fid : face_set[1]) 
    {
        F2idx(rowid++) = fid;
    }
}