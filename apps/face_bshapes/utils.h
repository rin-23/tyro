#pragma once 

#include <Eigen/Dense>
#define CSV_IO_NO_THREAD
#include <csv-parser/parser.hpp>
#include <vector>
#include <string>
#include <iostream>

namespace tyro 
{
    void csvToVector(const std::string& csv_file, 
                     std::vector<std::vector<double>>& mData,
                     int index_col = 0,
                     bool has_header = true,
                     int max_rows = 100) 
    {
        using namespace aria::csv;
        std::vector<std::string> mAttrs;
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
            if (row_idx > max_rows)
                break;
        }

        assert(mAttrs.size() == mData[0].size());

        /*
        for (auto& a : mData) 
        {
            for (auto&b : a) 
            {
                std::cout << b << " ";
            }
            std::cout <<"\n";
        }
        std::cout << row_idx << "\n";
        */
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
}
