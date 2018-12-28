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
#include "Wm5Transform.h"


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
        void load_oldman();
        //void load_bunny(bool serialized = true);
        
        void compute_average();
        
        void save_selected_faces(const std::string& filename);
        void save_selected_verticies(const std::string& filename);
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
        ES2VideoTextureSPtr m_video_texture2;
        
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
        bool isBLOBBY =false;
        //square selection
        int m_square_sel_start_x;
        int m_square_sel_start_y;
        bool m_computed_error = false;
        bool m_computed_vel_error = false;
        int m_frame_offset;
        SelectionType m_selection_type = SelectionType::Select;
        std::vector<SpatialSPtr> ball_list;
        std::atomic<int> m_frame;
        std::atomic<bool> m_need_rendering;
        //int  m_frame;   
        //bool m_need_rendering;
        double m_model_offset;
        //Eigen::MatrixXd jetColors;                
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
            std::vector<IGLMeshSPtr> errorVel;
            IGLMeshSPtr errorDeform;
            IGLMeshSPtr isoline;
            IGLMeshSPtr isolineSplit;
            IGLMeshWireframeSPtr isolineSplitWire;

            IGLMeshSPtr addShape1;
            IGLMeshSPtr addShape2;

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

        struct StopVideoCV 
        {
            std::vector<cv::Mat> F;
            std::vector<cv::Mat> D;
            Eigen::VectorXi L;
            
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

        StopVideoCV stop_video_cv;

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
            Eigen::VectorXd VW; //weight into stop motion for each vertex
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

        Eigen::MatrixXd VSPLIT;
        Eigen::MatrixXi FSPLIT;
        Eigen::MatrixXd NSPLIT;
        Eigen::MatrixXi UESPLIT;
        Eigen::MatrixXd UCSPLIT;
        Eigen::MatrixXd FCSPLIT;
        Eigen::VectorXd LABELS;
        Eigen::VectorXd ISOCOLORS;
        bool m_computed_iso_color = false;
        float max_error;
        float max_vel_error;
        float max_def_error;
        MAnimation ANIM; //Original animation data        
        MAnimation DANIM; // Animation data after we smooth the seam(s)
        std::vector<Eigen::MatrixXd> Lap;
        std::vector<MAnimation> PIECES; // Break deformed mesh into pieces along seam(s).
        std::vector<Eigen::VectorXi> PIECES_IDX;
        std::vector<MStopMotion> SMOTION; // Stop motion animate pieces
        std::vector<Eigen::MatrixXd> m_error;
        std::vector<Eigen::MatrixXd> m_error_velocity;
        Eigen::MatrixXd m_error_deform;
        //float max_error;
        //float max_error_velocity;
        Wm5::Transform globalTrans;
        
        //float max_error_deform;
            
        std::vector<std::string> FOLDERS;
        //std::vector<std::string> FOLDERS_MONKA;
        float m_ball_size = 0.01;
        //using Shot = std::tuple<int, int>; // start and end frame
        //using Movie = std::vector<Shot>;  // collection of shots. Each shot are indicies into m_frame_data.v_data
        //Movie movie;            
        
        MAnimation addAnim1;
        MAnimation addAnim2;

        std::vector<int> vid_list;
        std::vector<int> vid_list_second;
        std::vector<int> vid_list_third;
        std::vector<int> vid_list2; //verteicies selected for part 1 and assign weight

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
        void addSphere(int vid, const Eigen::MatrixXd& V, Wm5::Vector4f color = Wm5::Vector4f(1,0,0,1), Wm5::Transform worldT = Wm5::Transform::IDENTITY);        
        void removeSpheres(std::vector<int> vids);
        void setFaceColor(int fid, bool selected);
        void setFaceColor(int fid, const Eigen::Vector3d& clr);
        void selectVertex(Eigen::Vector2f& mouse_pos, int mouse_button, int modifier);
        void selectVertexPart(Eigen::Vector2f& mouse_pos, int mouse_button, int modifier, int whichpart); 
    
        void DrawMeshes();
        int ShowMonkaMovieStop(); 
        int ShowBunnyMovieStop();
        int ShowBunnyMovieStop2();

        int PRINTEDStopAnimMovie(); 

        void add_face(int fid);
        void add_vertex(int vid);
        void add_vertex2(int vid);

    };


    //Convert vertex selection to edge selection.
    //in vid_list - list of vertex ids (ORDERED RIGHT NOW, ASSUME CLOSED LOOP)
    //out E - vid_list.size() by 2 matrix of edges
    void convert_vertex_to_edge_selection(const std::vector<int>& vid_list,
                                          const std::vector<int>& vid_list_second,
                                          const std::vector<int>& vid_list_third,
                                          const Eigen::MatrixXi& E, //all directed edges
                                          const Eigen::MatrixXi& uE, //all unique edges
                                          const Eigen::VectorXi& EMAP, // map from directed to unique edge index 
                                          bool isClosedLoop,
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