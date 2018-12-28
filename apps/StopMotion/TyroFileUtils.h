#pragma once
#include <ios>	
#include <iomanip>	
#include <sstream>
#include <iostream>
#include <fstream>

namespace tyro
{
std::string pad_zeros(int num, int width=5) 
{
  std::ostringstream out;
  out << std::internal << std::setfill('0') << std::setw(width) << num;
  return out.str();
}

bool obj_file_path_list(const std::string& folder, //folder containing obj files and objlist file 
                        const std::string& obj_list_file,   //list of obj file names
                        std::vector<std::string>& obj_files, 
                        int& num_files_read)  //array of full paths for obj files
{   
    std::string obj_list_path;
    if (folder.back() == '/') 
        obj_list_path = folder + obj_list_file;
    else 
        obj_list_path = folder + std::string("/") + obj_list_file;

    std::ifstream input(obj_list_path.c_str());
    if (input.fail())
    {
        RA_LOG_ERROR_ASSERT("Cannot find file %s", obj_list_file.c_str());
        return false;
    }

    std::string obj_file_name;
    num_files_read = 0;
    while (std::getline(input, obj_file_name))
    {
        if (obj_file_name[0] != '#') //skip comments
        {   
            std::string filename;
            if (folder.back() == '/') 
                filename = folder + obj_file_name;
            else 
                filename = folder + std::string("/") + obj_file_name;
            obj_files.push_back(filename);
            num_files_read++;
        }
    }
    input.close();

    return true;
}
}