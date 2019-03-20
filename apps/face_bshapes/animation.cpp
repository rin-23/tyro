#include "animation.h"
#define CSV_IO_NO_THREAD
#include <csv-parser/parser.hpp>
#include <iostream>
#include "RAEnginePrerequisites.h"

using namespace std;

namespace tyro
{

// parse csv file
void Animation::readPandasCsv(const std::string& csv_file, int index_col) 
{   
    using namespace aria::csv;

    mAttrs.clear();
    mData.clear();

    std::ifstream f(csv_file);
    CsvParser parser(f);// do stuff with the data
    
    int row_idx = 0;
    for (auto& row : parser) 
    {   
        int col_idx = 0;
        std::vector<double> row_data;
        for (auto& field : row)
        { 
            if (col_idx != index_col) 
            {
                if (row_idx == 0)
                    mAttrs.push_back(field);
                else
                    row_data.push_back(stod(field));
            }
            col_idx++;
        }

        if (row_idx != 0)
            mData.push_back(row_data);

        row_idx++;
        //std::cout << std::endl;
    }

    if (mAttrs.size() != mData[0].size()) 
    {
        RA_LOG_ERROR_ASSERT("Number of attributes and data columns should be same");
    }

    //populate eigen matrix
    /*
    int rows = data.size();
    int cols = data[0].size();

    m_data.resize(rows,cols);
    for (int i = 0; i<rows; ++i) 
    {
        m_data(i,j) = data[i][j]
    }
    */
}


void Animation::readOpenFaceCsv(const std::string& csv_file, int index_col) 
{
    using namespace aria::csv;

    mAttrs.clear();
    mData.clear();

    std::ifstream f(csv_file);
    CsvParser parser(f);// do stuff with the data
    
    int row_idx = 0;
    for (auto& row : parser) 
    {   
        int col_idx = 0;
        std::vector<double> row_data;
        for (auto field : row)
        { 
            if (col_idx != index_col) 
            {
                if (row_idx == 0) 
                {
                    const char* t = " \t\n\r\f\v";
                    field.erase(0, field.find_first_not_of(t)); // this is dumb
                    mAttrs.push_back(field);
                }
                else
                    row_data.push_back(stod(field));
            }
            col_idx++;
        }

        if (row_idx != 0)
            mData.push_back(row_data);

        row_idx++;
        //std::cout << std::endl;
    }

    if (mAttrs.size() != mData[0].size()) 
    {
        RA_LOG_ERROR_ASSERT("Number of attributes and data columns should be same");
    }

    //populate eigen matrix
    /*
    int rows = data.size();
    int cols = data[0].size();

    m_data.resize(rows,cols);
    for (int i = 0; i<rows; ++i) 
    {
        m_data(i,j) = data[i][j]
    }
    */
}


//get weights for frame
void Animation::getWeights(int frame, std::vector<std::string>& attrs, std::vector<double>& weights)
{
    if (frame < mData.size()) 
    {
        weights = mData[frame];
        attrs = mAttrs;
    }
    else
    {
        RA_LOG_ERROR_ASSERT("frame is out of bounds");
    }
}     

int Animation::getNumFrames() 
{
    return mData.size();
}

/*
int Animation::setCustomAttrs(const std::vector<std::string>& attrs)
{
    assert(mAttrs.size() == attrs.size());
    RA_LOG_INFO("changing attributes names:");
    for (int i=0; i < mAttrs.size();++i) 
    {
        RA_LOG_INFO("changing %s to %s", mAttrs[i].c_str(), attrs[i].c_str());
    }
    mAttrs.clear();
    std::copy(attrs.begin(), attrs.end(), back_inserter(mAttrs)); 
}
*/

}