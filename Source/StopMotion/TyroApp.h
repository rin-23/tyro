#pragma once

#include "TyroWindow.h"
#include "RAES2Renderer.h"
#include "RAiOSCamera.h"
#include "Console.h"
#include <vector>
#include <Eigen/Core>
#include "TyroTimeLine.h"
#include <atomic>

#include "opencv_serialisation.h"
#include "eigen_cerealisation.h"
#include <cereal/types/vector.hpp>
#include <cereal/cereal.hpp>
#include <opencv2/opencv.hpp>

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
        int Video();
        int VideoToImages();
        int ParseImages();

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
        void debug_show_faces_near_edge_selection(const Eigen::VectorXi& uEI, const Eigen::VectorXi& DMAP);
        
        State m_state;
        SelectionPrimitive m_sel_primitive;
        SelectionMethod m_sel_method;
        
        Timeline* m_timeline;
        Window* m_tyro_window;
        ES2Renderer* m_gl_rend;
        iOSCamera* m_camera;
        ES2TextOverlaySPtr m_frame_overlay;
        ES2VideoTextureSPtr m_video_texture;
        
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
        bool m_show_wire;
        bool add_seg_faces;
        //square selection
        int m_square_sel_start_x;
        int m_square_sel_start_y;
        bool m_computed_error = false;
        int m_frame_offset;

        std::vector<SpatialSPtr> ball_list;
        std::atomic<int> m_frame;
        std::atomic<bool> m_need_rendering;
        //int  m_frame;   
        //bool m_need_rendering;
        double m_model_offset;
        
        struct MRenderData 
        {
            IGLMeshSPtr mesh;
            IGLMeshWireframeSPtr mesh_wire;
            bool org_visible = true;

            IGLMeshSPtr avg;
            IGLMeshWireframeSPtr avg_wire;
            bool avg_visible = true;

            IGLMeshSPtr dfm;
            IGLMeshWireframeSPtr dfm_wire;            
            bool dfm_visible;

            IGLMeshWireframeSPtr seam[2];            
            IGLMeshWireframeSPtr seam_on_main;            
            
            std::vector<IGLMeshSPtr> stop;
            std::vector<IGLMeshWireframeSPtr> stop_wire;
            bool stop_motion_visible = true;

            std::vector<IGLMeshSPtr> part;
            std::vector<IGLMeshWireframeSPtr> part_wire;
            bool parts_visible = true;

            

            std::vector<IGLMeshSPtr> error;
            //std::vector<IGLMeshWireframeSPtr> stop_motion_meshes_wire;
            
        };
        MRenderData RENDER;

        struct VideoCV 
        {
            std::vector<cv::Mat> F;
            template<class Archive>
            void save(Archive & archive) const
            {
                archive(F);
             
            }
            
            template<class Archive>
            void load(Archive & archive)
            {
                archive(F);
          
            }
        };
        VideoCV m_video;

        struct MAnimation 
        {   
            std::string unique_name;  // not unique in our implementation. used to search in array
            std::vector<Eigen::MatrixXd> VD; // Vertex data. 3*num_vert by num_frames. 
            std::vector<Eigen::MatrixXd> ND; // Normal data. 3*num_vert by num_frames. 
            Eigen::MatrixXi F; // Face data. 
            Eigen::MatrixXi E; // Edge data.
            Eigen::MatrixXi UE; // Uniqie edge data
            Eigen::VectorXi EMAP; // Map directed edges to unique edges. 
            Eigen::MatrixXd FC; // Face Color data
            Eigen::MatrixXd UEC; // Edge color data
            Eigen::MatrixXd AvgVD; // average of v_data
            std::vector<int> SIdx; //
            std::vector<Eigen::VectorXd> AO;
            template<class Archive>
            void save(Archive & archive) const
            {
                archive(VD);
                archive(ND);
                archive(F);
                archive(E);
                archive(UE);
                archive(EMAP);
                archive(FC);
                archive(UEC);
                archive(AvgVD);
                archive(SIdx);
            }
            
            template<class Archive>
            void load(Archive & archive)
            {
                archive(VD);
                archive(ND);
                archive(F);
                archive(E);
                archive(UE);
                archive(EMAP);
                archive(FC);
                archive(UEC);
                archive(AvgVD);
                archive(SIdx);            
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


        struct PrimitiveWeights 
        {
            //VectorXd V;
            Eigen::VectorXd FW; //face weights same size as F.rows
            Eigen::VectorXd VW; //vertex weights same size as V.rows
        };
        PrimitiveWeights m_weights;

        MAnimation ANIM; //Original animation data        
        MAnimation DANIM; // Animation data after we smooth the seam(s)
        std::vector<MAnimation> PIECES; // Break deformed mesh into pieces along seam(s).
        std::vector<MStopMotion> SMOTION; // Stop motion animate pieces
        std::vector<std::vector<Eigen::VectorXd>> m_error;
        float max_error;
        std::vector<std::string> FOLDERS;
        //std::vector<std::string> FOLDERS_MONKA;
        
        //using Shot = std::tuple<int, int>; // start and end frame
        //using Movie = std::vector<Shot>;  // collection of shots. Each shot are indicies into m_frame_data.v_data
        //Movie movie;            
        
        std::vector<int> vid_list;
        std::vector<int> fid_list; //fid added with left mouse click
        std::vector<int> fid_list2; //fid added with right mouse click
        std::vector<int> fid_list3; //fid added with right mouse click
        
        std::vector<std::vector<int>> m_eid_list; //seam on stop motion
        std::vector<int> m_eid_list2; //seam on regular model
        bool m_show_seam = false;

        void register_console_function(const std::string& name,
                                       const std::function<void(App*, const std::vector<std::string>&)>& con_fun,
                                       const std::string& help_txt);
        void load_mesh_sequence(const std::vector<std::string>& obj_list, bool use_igl_loader = true); 
        void update_camera();
        void render();
        void addSphere(int vid, Wm5::Vector4f color = Wm5::Vector4f(1,0,0,1));        
        void removeSpheres(std::vector<int> vids);
        void setFaceColor(int fid, bool selected);
        void setFaceColor(int fid, const Eigen::Vector3d& clr);
        void selectVertex(Eigen::Vector2f& mouse_pos, int mouse_button, int modifier);
        void DrawMeshes();

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