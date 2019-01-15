#pragma once

// per frame animation data
#include <string>
#include <Eigen/Dense>
#include <vector>

namespace tyro
{
class Animation 
{
public:
    //load animation from csv file
    void readPandasCsv(const std::string& csv_file, int index_col=-1);
    
    //get weights for frame
    void getWeights(int frame, std::vector<std::string>& attrs, std::vector<double>& weights);

    int getNumFrames();

    //int setCustomAttrs(const std::vector<std::string>& attrs);
private:
    std::vector<std::string> mAttrs; // list of attributes/bshapes names
    std::vector<std::vector<double>> mData; // #frame by #attributes matrix
};
}