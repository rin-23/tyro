#include "kdtree.h"
#include <cstdlib>
#include <ctime>
#include <iostream>


//using namespace Eigen;
using namespace std;
using namespace nanoflann;

namespace tyro 
{
    KDTree::KDTree() 
    {}
    
    KDTree::~KDTree()
    {}
    
    void KDTree::InitWithData(const std::vector<std::vector<double>>& data)
    {   
        mDim = data[0].size();
        mNumPoints = data.size();
        mat_index = new my_kd_tree_t(mDim, data, 10 /* max leaf */);
        mat_index->index->buildIndex();
   }
    
    void KDTree::FindClosest(const std::vector<double>& point, /*std::vector<double>& c_point,*/ double& c_dist) 
    {   
        // Query point:
        std::vector<double> query_pt(mDim);
        for (size_t d = 0; d < mDim; d++)
            query_pt[d] = point[d];
        
        int num_results=1;
        // do a knn search
        vector<size_t> ret_indexes(num_results);
        vector<double> out_dists_sqr(num_results);

        nanoflann::KNNResultSet<double> resultSet(num_results);

        resultSet.init(&ret_indexes[0], &out_dists_sqr[0]);
        mat_index->index->findNeighbors(resultSet, &query_pt[0],nanoflann::SearchParams(10));
        /*
        std::cout << "knnSearch(nn=" << num_results << "): \n";
        for (size_t i = 0; i < num_results; i++)
            std::cout << "ret_index[" << i << "]=" << ret_indexes[i]
                    << " out_dist_sqr=" << out_dists_sqr[i] << endl;
        */
        c_dist = out_dists_sqr[0];
        
    }

    void KDTree::Serialize(const std::string& path)
    {

    }
    
    void KDTree::Deserialize(const std::string& path)
    {

    }
}