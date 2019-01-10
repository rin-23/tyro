#include "load_mesh_sequence.h"
#include "RAEnginePrerequisites.h"
#include <igl/readOBJ.h>
#include <igl/per_vertex_normals.h>
#include <igl/list_to_matrix.h>
#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cc
#include <tinyobjloader/tiny_obj_loader.h>
#include <igl/unique_edge_map.h>
#include "RALogManager.h"
#include <igl/upsample.h>

namespace tyro 
{   
    bool load_mesh_sequence(const std::vector<std::string>& obj_paths,
                            std::vector<Eigen::MatrixXd>& v_data, 
                            std::vector<Eigen::MatrixXd>& n_data,  
                            Eigen::MatrixXi& f_data,
                            Eigen::MatrixXi& e_data,
                            Eigen::MatrixXi& ue_data,
                            Eigen::VectorXi& EMAP,
                            bool use_igl_loader) 
    {   
        if (use_igl_loader) 
        {            
            bool saved_faces = false;
            for (auto& line : obj_paths)
            {  
                //RA_LOG_INFO("Reading file %s", line.data());
                if (line[0] == '#') //skip comments
                    continue;

                Eigen::MatrixXd oldV;
                Eigen::MatrixXi oldF;
                igl::readOBJ(line, oldV, oldF);
                
                //upsample
                Eigen::SparseMatrix<double> S;
                Eigen::MatrixXi F;
                igl::upsample(oldV.rows(), oldF, S, F);
                Eigen::MatrixXd V = (S*oldV).eval();
                
                Eigen::MatrixXd N;
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

                std::vector<std::vector<int> > uE2E;
                igl::unique_edge_map(F,e_data,ue_data,EMAP,uE2E);
            }
        } 
        else //use tinyobjloader
        {   
            bool saved_faces = false;
            //std::string shape_name("GEO-Agent_head");
            std::string shape_name("");
            for (auto& line : obj_paths)
            {
                //std::string inputfile = line;
                RA_LOG_INFO("Reading file %s", line.data());
                tinyobj::attrib_t attrib;
                std::vector<tinyobj::shape_t> shapes;
                std::vector<tinyobj::material_t> materials;
                
                std::string err;
                bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, line.c_str(), NULL, true);
                
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
                    //RA_LOG_INFO("Shape name %s", shapes[s].name.data());
                    
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
                

                std::vector<std::vector<int> > uE2E;
                igl::unique_edge_map(K,e_data,ue_data,EMAP,uE2E);
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


    bool load_meshes(const std::vector<std::string>& obj_paths,
                     std::vector<Eigen::MatrixXd>& v_data, 
                     std::vector<Eigen::MatrixXd>& n_data,  
                     Eigen::MatrixXi& F) 
    {           
        for (const auto& p : obj_paths)
        {
            Eigen::MatrixXd V,N;
            load_mesh(p,V,N,F);
            v_data.push_back(V);
            n_data.push_back(N);
        }
    }  

    bool load_mesh(const std::string& obj_path,
                Eigen::MatrixXd& V, 
                Eigen::MatrixXd& N,  
                Eigen::MatrixXi& F) 
    {
        Eigen::MatrixXi E, UE;
        Eigen::VectorXi EMAP;
        return load_mesh(obj_path,V,N,F,E,UE,EMAP);
    }

    bool load_mesh(const std::string& obj_path,
                    Eigen::MatrixXd& V, 
                    Eigen::MatrixXd& N,  
                    Eigen::MatrixXi& F,
                    Eigen::MatrixXi& E,
                    Eigen::MatrixXi& UE,
                    Eigen::VectorXi& EMAP)
    {       
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        
        std::string err;
        bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, obj_path.c_str(), NULL, true);
        
        if (!err.empty()) // `err` may contain warning message. 
        { 
            std::cerr << err << std::endl;
        }

        if (!ret) 
        {
            RA_LOG_ERROR_ASSERT("tiny obj loader asserted");
            return false;
        }

        // Loop over shapes
        std::vector<std::vector<double>> V_array;
        std::vector<std::vector<int>> F_array;

        // Generate F array
        int min_idx = INT_MAX;
        int max_idx = INT_MIN;
        for (size_t s = 0; s < shapes.size(); s++) 
        {
            // Loop over faces(polygon)
            size_t index_offset = 0;
            
            //if (shapes[s].name != shape_name)
            //    continue;

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
                index_offset += fv; //should be plus 3

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
        assert(igl::list_to_matrix(V_array, V));
        assert(igl::list_to_matrix(F_array, F));

        Eigen::MatrixXi F_ones;
        F_ones.resize(F.rows(), F.cols());
        F_ones.setConstant(min_idx);
        //F = F - F_ones; 
        
        igl::per_vertex_normals(V, F, N); 

        //std::vector<std::vector<int> > uE2E;
        //igl::unique_edge_map(F, E, UE, EMAP, uE2E);
        return true;
   }
}