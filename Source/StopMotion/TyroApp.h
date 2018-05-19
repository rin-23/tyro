#pragma once

#include "TyroWindow.h"
#include "RAES2Renderer.h"
#include "RAiOSCamera.h"
#include "Console.h"
#include <vector>
#include <Eigen/Core>
#include "TyroTimeLine.h"
#include <atomic>

#include "eigen_cerealisation.h"
#include <cereal/types/vector.hpp>
#include <cereal/cereal.hpp>

namespace tyro
{   
    class App 
    {
    public:

        enum State {None, Launched, LoadedModel, PlayingAnimation};
        enum SelectionPrimitive {Vertex, Faces, Edges};
        enum SelectionMethod {OneClick, Square};
        enum SelectionType {Select, Deselect};

        App();
        
        ~App();
        
        int Launch();

        //Commands
        void load_blobby();
        void load_oldman();
        void load_bunny(bool serialized = true);
        
        void compute_average();
        
        void save_selected_faces(const std::string& filename);
        void save_selected_verticies(const std::string& filename);
        void load_selected_faces(const std::string& filename);
        void load_selected_verticies(const std::string& filename); 
        void set_sel_primitive(App::SelectionPrimitive sel_state);
        void set_sel_method(App::SelectionMethod sel_state);
        void clear_all_selection();
        void invert_face_selection();
        void align_all_models();
        void align_all_models(int vid, Eigen::Vector3d offset);
        //void frame(int frame);
        //void show_edge_selection();
        void debug_show_faces_near_edge_selection(const Eigen::VectorXi& uEI, const Eigen::VectorXi& DMAP);
        

        State m_state;
        SelectionPrimitive m_sel_primitive;
        SelectionMethod m_sel_method;
        
        Timeline* m_timeline;
        Window* m_tyro_window;
        ES2Renderer* m_gl_rend;
        iOSCamera* m_camera;
        ES2TextOverlaySPtr m_frame_overlay;
        
        void mouse_down(Window& window, int button, int modifier);
        void mouse_up(Window& window, int button, int modifier);
        void mouse_move(Window& window, int mouse_x, int mouse_y);
        void window_resize(Window& window, unsigned int w, unsigned int h);    
        void key_pressed(Window& window, unsigned int key, int modifiers); 
        void key_down(Window& window, unsigned int key, int modifiers); 
        void mouse_scroll(Window& window, float ydelta); 
        
        bool mouse_is_down;
        int gesture_state; 
        int current_mouse_x;
        int current_mouse_y;
        bool show_console;        
        int m_modifier;
        int m_mouse_btn_clicked;
        bool m_computed_deformation;
        bool m_computed_avg;
        bool m_computed_stop_motion;
        bool m_computed_parts;
        Console m_console;
        bool m_update_camera;
        bool m_show_wireframe;
        bool add_seg_faces;
        //square selection
        int m_square_sel_start_x;
        int m_square_sel_start_y;
        
        std::vector<SpatialSPtr> ball_list;
        std::atomic<int> m_frame;
        
        double m_model_offset;
        
        struct MRenderData 
        {
            IGLMeshSPtr org_mesh;
            IGLMeshWireframeSPtr org_mesh_wire;
            bool org_visible = true;

            IGLMeshSPtr avg_mesh;
            IGLMeshWireframeSPtr avg_mesh_wire;
            bool avg_visible = true;

            IGLMeshSPtr dfm_mesh;
            IGLMeshWireframeSPtr dfm_mesh_wire;            
            bool dfm_visible;

            std::vector<IGLMeshSPtr> stop_motion_meshes;
            std::vector<IGLMeshWireframeSPtr> stop_motion_meshes_wire;
            bool stop_motion_visible = true;

            std::vector<IGLMeshSPtr> part_meshes;
            std::vector<IGLMeshWireframeSPtr> part_meshes_wire;
            bool parts_visible = true;
        };
        MRenderData render_data;

        struct MAnimation 
        {   
            std::string unique_name;  // not unique in our implementation. used to search in array
            std::vector<Eigen::MatrixXd> v_data; // Vertex data. 3*num_vert by num_frames. 
            std::vector<Eigen::MatrixXd> n_data; // Normal data. 3*num_vert by num_frames. 
            Eigen::MatrixXi f_data; // Face data. 
            Eigen::MatrixXi e_data; // Edge data.
            Eigen::MatrixXi ue_data; // Uniqie edge data
            Eigen::VectorXi EMAP; // Map directed edges to unique edges. 
            Eigen::MatrixXd fc_data; // Face Color data
            Eigen::MatrixXd ec_data; // Edge color data
            Eigen::MatrixXd avg_v_data; // average of v_data
            std::vector<int> sequenceIdx; //

            template<class Archive>
            void save(Archive & archive) const
            {
                archive(v_data);
                archive(n_data);
                archive(f_data);
                archive(e_data);
                archive(ue_data);
                archive(EMAP);
                archive(fc_data);
                archive(ec_data);
                archive(avg_v_data);
                archive(sequenceIdx);
            }
            
            template<class Archive>
            void load(Archive & archive)
            {
                archive(v_data);
                archive(n_data);
                archive(f_data);
                archive(e_data);
                archive(ue_data);
                archive(EMAP);
                archive(fc_data);
                archive(ec_data);
                archive(avg_v_data);
                archive(sequenceIdx);            
            }
        };

        struct MStopMotion 
        {   
            bool computed = false;
            MAnimation anim;
            std::vector<Eigen::MatrixXd> D; //dictionary of faces
            Eigen::VectorXi L; // Indicies into D to label frames in anim.v_data;

            template<class Archive>
            void save(Archive & archive) const
            {   
                //archive(computed);
                archive(anim);
                archive(D);
                archive(L);
            }
            
            template<class Archive>
            void load(Archive & archive)
            {   
                //archive(computed);
                archive(anim);
                archive(D);
                archive(L);
            }
        };

        MAnimation m_frame_data; //Original animation data        
        MAnimation m_frame_deformed_data; // Animation data after we smooth the seam(s)
        std::vector<MAnimation> m_pieces; // Break deformed mesh into pieces along seam(s).
        std::vector<MStopMotion> m_stop_motion; // Stop motion animate pieces
        std::vector<std::string> FOLDERS;
        //std::vector<std::string> FOLDERS_MONKA;
        
        //using Shot = std::tuple<int, int>; // start and end frame
        //using Movie = std::vector<Shot>;  // collection of shots. Each shot are indicies into m_frame_data.v_data
        //Movie movie;            
        
        std::vector<int> vid_list;
        std::vector<int> fid_list; //fid added with left mouse click
        std::vector<int> fid_list2; //fid added with right mouse click
        std::vector<int> fid_list3; //fid added with right mouse click
        
        std::vector<int> eid_list;

        void register_console_function(const std::string& name,
                                       const std::function<void(App*, const std::vector<std::string>&)>& con_fun,
                                       const std::string& help_txt);
        void load_mesh_sequence(const std::vector<std::string>& obj_list, bool use_igl_loader = true); 
        void update_camera();
        void render();
        void addSphere(int vid);        
        void removeSpheres(std::vector<int> vids);
        void setFaceColor(int fid, bool selected);
        void setFaceColor(int fid, const Eigen::Vector3d& clr);
        void selectVertex(Eigen::Vector2f& mouse_pos, int mouse_button, int modifier);
        
        std::atomic<bool> m_need_rendering;
    };


    //Convert vertex selection to edge selection.
    //in vid_list - list of vertex ids (ORDERED RIGHT NOW, ASSUME CLOSED LOOP)
    //out E - vid_list.size() by 2 matrix of edges
    void convert_vertex_to_edge_selection(const std::vector<int>& vid_list,
                                          const Eigen::MatrixXi& E, //all directed edges
                                          const Eigen::MatrixXi& uE, //all unique edges
                                          const Eigen::VectorXi& EMAP, // map from directed to unique edge index 
                                          Eigen::MatrixXi& eid_list, // edges from vid_list
                                          Eigen::VectorXi& EI, // indicies into directed edges matrix
                                          Eigen::VectorXi& uEI, // indicies into undirected edges matrix
                                          Eigen::VectorXi& DMAP); 

    // Create color matrix for unique edges from color vector
    //in rows - number of unique edges
    //in cv - color vector
    //out C - same size as uE. contains color per edge
    void color_matrix(int rows, const Eigen::Vector3d& cv, Eigen::MatrixXd& uC);
    void color_black_matrix(int rows, Eigen::MatrixXd& uC);
        
    void copy_animation(const App::MAnimation& source, 
                        App::MAnimation& dest, 
                        bool topology, 
                        bool face_color, 
                        bool edge_color);

}