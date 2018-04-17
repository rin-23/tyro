#include "stop_motion_data.h"
#include <iostream>
#include <fstream>
#include "RAEnginePrerequisites.h"
#include <igl/readOBJ.h>
#include <igl/per_vertex_normals.h>
#include <igl/list_to_matrix.h>
#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cc
#include "tiny_obj_loader.h"

using namespace tyro;

namespace stop 
{   
    bool load_mesh_sequence(const std::string& obj_list_file,
                            std::vector<Eigen::MatrixXd>& v_data, 
                            std::vector<Eigen::MatrixXd>& n_data,  
                            Eigen::MatrixXi& f_data,
                            bool use_igl_loader) 
    {   
        if (use_igl_loader) 
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
        else //use tinyobjloader
        {   
            std::ifstream input(obj_list_file.c_str());
            if (input.fail())
            {
                RA_LOG_ERROR_ASSERT("Cannot find file %s",obj_list_file);
                return false;
            }

            std::string line;
            bool saved_faces = false;
            std::string shape_name("rabbit");

            while (std::getline(input, line))
            {
                if (line[0] == '#') //skip comments
                    continue;

                std::string inputfile = line;
                tinyobj::attrib_t attrib;
                std::vector<tinyobj::shape_t> shapes;
                std::vector<tinyobj::material_t> materials;
                
                std::string err;
                bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, inputfile.c_str(), NULL, true);
                
                if (!err.empty()) { // `err` may contain warning message.
                std::cerr << err << std::endl;
                }

                if (!ret) {
                    RA_LOG_ERROR_ASSERT("tiny obj loader asserted");
                }

                // Loop over shapes
                std::vector<std::vector<double>> V_array;
                std::vector<std::vector<int>> F_array;

            

                //generate F array
                int min_idx = INT_MAX;
                int max_idx = INT_MIN;
                for (size_t s = 0; s < shapes.size(); s++) 
                {
                    // Loop over faces(polygon)
                    size_t index_offset = 0;
                    RA_LOG_INFO("Shape name %s", shapes[s].name.data());
                    
                    if (shapes[s].name != shape_name)
                        continue;

                    for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) 
                    {
                        int fv = shapes[s].mesh.num_face_vertices[f];
                        assert(fv == 3);

                        tinyobj::index_t idx1 = shapes[s].mesh.indices[index_offset + 0];
                        tinyobj::index_t idx2 = shapes[s].mesh.indices[index_offset + 1];
                        tinyobj::index_t idx3 = shapes[s].mesh.indices[index_offset + 2];
                            
                        std::vector<int> ftx = {idx1.vertex_index,
                                                idx2.vertex_index,
                                                idx3.vertex_index};
                        F_array.push_back(ftx);
                        index_offset += fv; //should plus 3

                        min_idx = std::min(min_idx, idx1.vertex_index);
                        min_idx = std::min(min_idx, idx2.vertex_index);
                        min_idx = std::min(min_idx, idx3.vertex_index);

                        max_idx = std::max(max_idx, idx1.vertex_index);
                        max_idx = std::max(max_idx, idx2.vertex_index);
                        max_idx = std::max(max_idx, idx3.vertex_index);
                    }
                }   

                //generate V array by slicing the original
                for (int i = 0; i < attrib.vertices.size(); i+=3) 
                {   
                    int actual_indx = i / 3;
                    if (actual_indx >= min_idx && actual_indx <= max_idx) 
                    {
                        std::vector<double> vtx = {attrib.vertices[i+0], 
                                                   attrib.vertices[i+1], 
                                                   attrib.vertices[i+2]};
                        V_array.push_back(vtx);
                    }
                }

                //flatten them 
                Eigen::MatrixXd V, N;
                Eigen::MatrixXi F;
                bool V_rect = igl::list_to_matrix(V_array,V);
                bool F_rect = igl::list_to_matrix(F_array,F);
                assert(V_rect);
                assert(F_rect);

                Eigen::MatrixXi F_ones;
                F_ones.resize(F.rows(), F.cols());
                F_ones.setConstant(min_idx);
                Eigen::MatrixXi K = F - F_ones; 
                
                igl::per_vertex_normals(V, K, N); 

                v_data.push_back(V);
                n_data.push_back(N);
                
                //all meshes in the sequence share same face data
                if (!saved_faces) 
                {   
                    //auto F_slice = F.topRows(6000);
                    f_data = K;
                    saved_faces = true;
                }
            }
        }
    }
    /*
    bool load_mesh_sequence(const std::string& obj_list_file,
                            Eigen::MatrixXd v_data, 
                            Eigen::MatrixXd n_data,  
                            Eigen::MatrixXi f_data)
    {
        assert(false);
    }
    */
}