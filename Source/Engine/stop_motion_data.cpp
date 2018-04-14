#include "stop_motion_data.h"
#include <iostream>
#include <fstream>
#include "RAEnginePrerequisites.h"
#include <igl/readOBJ.h>
#include <igl/per_vertex_normals.h>

using namespace tyro;

namespace stop 
{   
    bool load_mesh_sequence(const std::string& obj_list_file,
                            std::vector<Eigen::MatrixXd>& v_data, 
                            std::vector<Eigen::MatrixXd>& n_data,  
                            Eigen::MatrixXi& f_data) 
    {
        std::ifstream input(obj_list_file.c_str());
        if (input.fail())
        {
            RA_LOG_ERROR_ASSERT("Cannot find file %s",obj_list_file);
            return false;
        }

    	std::string line;
        bool saved_faces = false;
        while (std::getline(input, line))
	    {
            if (line[0] == '#') //skip comments
                continue;

            Eigen::MatrixXd V, N;
            Eigen::MatrixXi F;
            igl::readOBJ(line, V, F);
            int num_face = F.rows();
            igl::per_vertex_normals(V, F, N); 

            v_data.push_back(V);
            n_data.push_back(N);
            
            //all meshes in the sequence share same face data
            if (!saved_faces) 
            {
                f_data = F;
                saved_faces = true;
            }
        }
    }

    bool load_mesh_sequence(const std::string& obj_list_file,
                            Eigen::MatrixXd v_data, 
                            Eigen::MatrixXd n_data,  
                            Eigen::MatrixXi f_data)
    {
        assert(false);
    }
}