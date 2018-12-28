#include "TyroVJApp.h"
#include "RAEnginePrerequisites.h"
#include "RAES2TextOverlay.h"
#include "RAFont.h"
#include "RAVisibleSet.h"
//#include "RAES2StandardMesh.h"
//#include "RAAxisAlignedBBox.h"
#include <stdio.h>
#include <functional>
#include "Wm5APoint.h"
#include "Wm5Vector2.h"
#include "Wm5Vector4.h"
//#include "TyroIGLMesh.h"
//#include "TyroFileUtils.h"
#include <random>

using namespace std;

using Wm5::APoint;

namespace tyro
{   
    App::App()
    :
    m_tyro_window(nullptr),
    m_gl_rend(nullptr),
    m_camera(nullptr),
    mouse_is_down(false),
    gesture_state(0),
    show_console(false),
    //m_frame(0),
    m_state(App::State::None),
    m_need_rendering(false),
    m_computed_deformation(false),
    m_computed_avg(false),
    //m_sel_primitive(App::SelectionPrimitive::Faces),
    //m_sel_method(App::SelectionMethod::OneClick),
    m_computed_stop_motion(false),
    m_update_camera(false),
    m_frame_overlay(nullptr),
    m_computed_parts(false),
    m_show_wire(true)
    //add_seg_faces(false),
    //m_video_texture(nullptr),
    //m_frame_offset(0)
    {}

    App::~App() 
    {
        if (m_tyro_window)
        {
            m_tyro_window->Terminate();
            delete m_tyro_window; 
        }

        if (m_gl_rend) 
            delete m_gl_rend;
        
        if (m_camera)
            delete m_camera;
    }
    
    int App::Launch()
    {   
        RA_LOG_INFO("Launching the app");

        //setup windowshapes
        m_tyro_window = new Window();
        m_tyro_window->Init();
                
        //setup renderer
        m_gl_rend = new ES2Renderer(m_tyro_window->GetGLContext());
        m_gl_rend->SetClearColor(Wm5::Vector4f(0, 1, 1, 1));

        int v_width, v_height;
        m_tyro_window->GetGLContext()->getFramebufferSize(&v_width, &v_height);
        Wm5::Vector4i viewport(0, 0, v_width, v_height);
        m_camera = new iOSCamera(Wm5::APoint(0,0,0), 1.0, 1.0, 2, viewport, true);
        
        /* m_timeline = new Timeline(40, 300);
        m_timeline->frameChanged = [&](Timeline& timeline, int frame)->void 
        {   
            //RA_LOG_INFO("Frame Change BEGIN");
            m_frame = frame;
            m_need_rendering = true;
            glfwPostEmptyEvent();
            //RA_LOG_INFO("Frame Change END");
        };
        */
       
        //set up window callbacks
        //@TODO use std::bind instead
        m_tyro_window->callback_mouse_down = [&](Window& window, int button, int modifier)->bool 
        {
            this->mouse_down(window, button, modifier);
        };

        m_tyro_window->callback_mouse_up = [&](Window& window, int button, int modifier)->bool 
        {
            this->mouse_up(window, button, modifier);
        };

        m_tyro_window->callback_mouse_move = [&](Window& window, int mouse_x, int mouse_y)->bool 
        {
            this->mouse_move(window, mouse_x, mouse_y);
        };

        m_tyro_window->callback_window_resize = [&](Window& window, unsigned int w, unsigned int h)->bool 
        {
            this->window_resize(window, w, h);
        };

        m_tyro_window->callback_key_pressed = [&](Window& window, unsigned int key, int modifiers)->bool 
        {
            this->key_pressed(window, key, modifiers);
        };

        m_tyro_window->callback_key_down = [&](Window& window, unsigned int key, int modifiers)->bool 
        {
            this->key_down(window, key, modifiers);
        };
        
        m_tyro_window->callback_mouse_scroll = [&](Window& window, float ydelta)->bool 
        {
            this->mouse_scroll(window, ydelta);
        };
        
      /*   register_console_function("load_jali", console_load_jali, "");
        register_console_function("save_avg", console_save_deform_average, "");
        register_console_function("write_labels", console_write_labels, "");
        register_console_function("compute_stop_frames", console_compute_stop_frames, "");
        register_console_function("uniform", console_uniform, "");
        register_console_function("save_few_deform_frames", console_save_few_deform_frames, "");
        register_console_function("save_first_frame", console_save_first_frame, "");
        register_console_function("compute_alec", console_compute_alec,"");

        register_console_function("compute_random_swith", console_figure_random, "");
        register_console_function("compute_data_vel_error", console_compute_data_vel_error,"");
        register_console_function("compute_errors", console_compute_errors,"");
        register_console_function("plot_error_deform", console_plot_error_deform, "");
        register_console_function("clear_weights", console_clear_weights, "");
        register_console_function("segmentation_full", console_segmentation_full, "");
        register_console_function("print_stats", console_print_stats, "");
        register_console_function("set_selection_type", console_set_selection_type, "");
        register_console_function("mesh_split_final", console_mesh_split_final, "");
        register_console_function("compute_deformation2", console_compute_deformation2, "");
        register_console_function("seam_from_selection", console_seam_from_selection, "");
        register_console_function("add_adjaent_boundary", console_add_adjaent_boundary, "");
        register_console_function("fix_boundary_edges", console_fix_boundary_edges, "");
        register_console_function("remove_duplicate_vertices", console_remove_duplicate_vertices, "");
        register_console_function("show_boundary_faces", console_show_boundary_faces, "");
        register_console_function("find_boundary", console_find_boundary, "");
        register_console_function("mesh_split_bndr", console_mesh_split_bndr, "");
        register_console_function("compute_vertex_bndr", console_compute_vertex_bndr, "");
        register_console_function("load_agent", console_load_agent, "");
        register_console_function("load_bartender", console_load_bartender, "");
        register_console_function("load_blobby", console_load_blobby, "");
        register_console_function("load_oldman", console_load_oldman, "");
        register_console_function("load_bunny", console_load_bunny, "");
        register_console_function("compute_average", console_compute_average, "");
        register_console_function("compute_deformation", console_compute_deformation, "");
        register_console_function("save_selected_faces", console_save_selected_faces, "");
        register_console_function("load_selected_faces", console_load_selected_faces, "");
        register_console_function("save_selected_verticies", console_save_selected_verticies, "");
        register_console_function("load_selected_verticies", console_load_selected_verticies, "");
        register_console_function("invert_face_selection", console_invert_face_selection, "");
        register_console_function("set_sel_primitive", console_set_sel_primitive, "");
        register_console_function("set_sel_method", console_set_sel_method, "");
        register_console_function("save_mesh_sequence_with_selected_faces", console_save_mesh_sequence_with_selected_faces, "");
        register_console_function("align_all_models", console_align_all_models, "");
        register_console_function("stop_motion", console_stop_motion, "");
        register_console_function("frame", console_frame, "");
        //register_console_function("split_mesh", console_split_mesh, "");
        register_console_function("show_edge_selection", console_show_edge_selection, "");
        register_console_function("deselect_faces", console_deselect_faces, "");
        register_console_function("deselect_verticies", console_deselect_verticies, "");
        register_console_function("show_wireframe", console_show_wireframe, "");
        register_console_function("segmentation", console_segmentation, "");
        register_console_function("clear_selection", console_clear_selection, "");
        register_console_function("clear_vertex_selection", console_clear_vertex_selection, "");
        register_console_function("clear_face_selection", console_clear_face_selection, "");

        register_console_function("save_serialised_data", console_save_serialised_data, "");
        register_console_function("load_serialised_data", console_load_serialised_data, "");
        register_console_function("load_monka", console_load_monka, "");
        register_console_function("load_monka_print", console_load_monka_print, "");
        
        register_console_function("set_vertex_weight", console_set_vertex_weight, "");
        register_console_function("set_face_weight", console_set_face_weight, "");
        
        register_console_function("draw_vertex_weight_map", console_draw_vertex_weight_map, "");
        register_console_function("draw_face_weight_map", console_draw_face_weight_map, "");
        register_console_function("upsample", console_upsample, "");
        register_console_function("taubin_smooth_along_edges", console_taubin_smooth_along_edges, "");
        register_console_function("laplacian_smooth_vert", console_laplacian_smoothing_vert, "");

        register_console_function("laplacian_smooth_along_edges", console_laplacian_smooth_along_edges, "");
        register_console_function("console_offset_frame", console_offset_frame, "");
        register_console_function("plot_error", console_plot_error, "");
        register_console_function("plot_error_vel", console_plot_error_vel, "");

        register_console_function("show_seam", console_show_seam, ""); 
        register_console_function("render_to_image", console_render_to_image, ""); 
        register_console_function("render_to_video", console_render_to_video, ""); 

        register_console_function("save_sub_split", console_save_sub_split, "");
        register_console_function("save_sub_animation", console_save_sub_animation, "");
        register_console_function("save_sub_deformation", console_save_sub_deformation, "");

        register_console_function("save_for_printing", console_save_for_printing, "");
        register_console_function("save_sub_video", console_save_sub_video, "");
        register_console_function("load_bunny_stop", console_load_bunny_stop, "");
        register_console_function("load_bunny_stop2", console_load_bunny_stop2, "");

        register_console_function("show_iso", console_show_iso, "");
        register_console_function("show_iso_2", console_show_iso_2, "");

        register_console_function("show_label", console_show_labels, "");
        register_console_function("scale", console_scale, "");
        register_console_function("collapse_small_triangles", console_collapse_small_triangles, "");
 */

        m_state = App::State::Launched;
        // Loop until the user closes the window
       // m_tyro_window->GetGLContext()->swapBuffers();
        
        m_need_rendering = true;
        //render(); //do initial render

        //while (!m_tyro_window->ShouldClose()) 
       // {
            //RA_LOG_INFO("Looping GLFW");
        //    m_tyro_window->Wait();
        //}
        FontManager* fManager = FontManager::GetSingleton();
        float scale = 1;
        float ppi = 144;
        fManager->Setup(ppi, scale);

        ES2FontSPtr font = FontManager::GetSingleton()->GetSystemFontOfSize12();
        std::string strrr("Framasdasdasdsaddasde 0/9000");
        m_frame_overlay = ES2TextOverlay::Create(strrr, 
                                                 Wm5::Vector2f(0, 0), 
                                                 font, 
                                                 Wm5::Vector4f(0,0,1,1), 
                                                 viewport);
                                                 
        m_frame_overlay->SetTranslate(Wm5::Vector2i(-viewport[2]/2 ,-viewport[3]/2 ));
        m_frame_overlay->SetText(strrr);

        //ParseImages();
        m_shaderbox = ShaderBox::Create();

        while (!m_tyro_window->ShouldClose())
        {   
            if (m_need_rendering) 
            {
                if (m_state == App::State::Launched) 
                {
                    m_gl_rend->ClearScreen();
               // }
                //else if (m_state == App::State::LoadedModel) 
                //{   
                    DrawMeshes();
                }
                
                // Draw console
                if (show_console) 
                {
                    glUseProgram(0);
                    m_console.display(2);
                }
                // Poll for and process events
                m_tyro_window->GetGLContext()->swapBuffers();
                m_need_rendering = false;             
            }
            
            m_tyro_window->Wait();
            //m_tyro_window->ProcessUserEvents();

        }

	    return 0;
    }

    void App::DrawMeshes() 
    {
        //RA_LOG_INFO("RENDER BEGIN");

        //create renderable for mesh
        VisibleSet vis_set;
   
        //std::string fstr = std::string("Frame ") + std::to_string(m_frame) + std::string("/") + std::to_string(ANIM.VD.size());
        //fstr =fstr+ std::string(" Scene ") + std::to_string(scene_id);
        //m_frame_overlay->SetText(fstr);
        vis_set.Insert(m_frame_overlay.get());
        vis_set.Insert(m_shaderbox.get());
        if (m_update_camera) 
        {
            update_camera();
            m_update_camera = false;
        }
        
        m_gl_rend->RenderVisibleSet(&vis_set, m_camera);       
    }

    void App::render() 
    {   
        //RA_LOG_INFO("NEED RENDERING");
        m_need_rendering = true;
    }

    void App::update_camera() 
    {
        //setup camera
        //AxisAlignedBBox WorldBoundBox = RENDER.mesh->WorldBoundBox;
        //Wm5::APoint world_center = WorldBoundBox.GetCenter();
        //float radius = std::abs(WorldBoundBox.GetRadius()*1.5);
        Wm5::APoint world_center = Wm5::APoint::ORIGIN;
        float radius = 1;
        int v_width, v_height;
        m_tyro_window->GetGLContext()->getFramebufferSize(&v_width, &v_height);
        Wm5::Vector4i viewport(0, 0, v_width, v_height);
        float aspect = (float)v_width/v_height;
        
        if (m_camera)
            delete m_camera;
        
        m_camera = new iOSCamera(world_center, radius, aspect, 2, viewport, true);
    }

    void App::mouse_down(Window& window, int button, int modifier) 
    {   
        //RA_LOG_INFO("mouse down %i", button);

        if (m_state != App::State::LoadedModel) return;

        mouse_is_down = true;
        m_modifier = modifier;
        m_mouse_btn_clicked = button;

        if (m_modifier == TYRO_MOD_CONTROL) return; //rotating
        if (button == 0 && m_modifier == TYRO_MOD_SHIFT) 
        {   
            //m_square_sel_start_x = current_mouse_x;
            //m_square_sel_start_y = current_mouse_y;
            return; //selection
        }
        if (m_mouse_btn_clicked == 2) return; //translating
        
        // Cast a ray in the view direction starting from the mouse position
        double x = current_mouse_x;
        double y = m_camera->GetViewport()[3] - current_mouse_y;
        //Eigen::Vector2f mouse_pos(x,y);
        //selectVertex(mouse_pos, button, modifier);     
    }

    void App::mouse_up(Window& window, int button, int modifier) 
    {   
        if (m_state != App::State::LoadedModel) return;
    }

    void App::mouse_move(Window& window, int mouse_x, int mouse_y) 
    {   
        //RA_LOG_INFO("mouse move state %i", m_state);
        if (m_state != App::State::LoadedModel) return;
        
        current_mouse_x = mouse_x;
        current_mouse_y = mouse_y;        
    }

    void App::mouse_scroll(Window& window, float ydelta) 
    {
        //RA_LOG_INFO("mouse scroll delta %f", ydelta);
        if (m_state != App::State::LoadedModel) return;
        
        m_camera->HandlePinchGesture(gesture_state, Wm5::Vector2i(current_mouse_x, current_mouse_y), ydelta);
        render();
    } 

    void App::window_resize(Window& window, unsigned int w, unsigned int h)
    {
        //RA_LOG_INFO("window resized")
        //float  aspect = m_gl_rend->GetViewWidth()/ (float) m_gl_rend->GetViewHeight();
        //Vector4i viewport(0, 0, m_gl_rend->GetViewWidth(), m_gl_rend->GetViewHeight());
        //m_camera->SetAspect(aspect);
        //m_camera->SetViewport(viewport);
        render();
    }


    void App::key_pressed(Window& window, unsigned int key, int modifiers) 
    {   
        //RA_LOG_INFO("Key pressed %c", key);
        
        if (key == '`') 
        {   
            //RA_LOG_INFO("Pressed %c", key);
            show_console = !show_console;
            render();
            return;
        }

        if (show_console) 
        {  
           m_console.keyboard(key);
           render();
           return;
        }
        else 
        {
            
        }
    }
    
    void App::key_down(Window& window, unsigned int key, int modifiers) 
   {   
        //RA_LOG_INFO("Key down %i", key)
        // handle not text keys   

        if (key == TYRO_KEY_LEFT){ 
            m_console.key_left();
            render();}
        else if (key == TYRO_KEY_RIGHT){ 
            m_console.key_right();
            render();}
        else if (key == TYRO_KEY_ENTER){
            m_console.key_enter();
            render();
            }
        else if (key == TYRO_KEY_BACKSPACE){
            m_console.key_backspace();render();}
        else if (key == TYRO_KEY_UP){
            m_console.key_up();render();}
        else if (key == TYRO_KEY_DOWN){
            m_console.key_down();render();}
        else if (key == TYRO_KEY_TAB){
            m_console.key_tab();render();}
    }

    void App::register_console_function(const std::string& name,
                                         const std::function<void(App*, const std::vector<std::string>&)>& con_fun,
                                         const std::string& help_txt)
    {
        std::function<void (const std::vector<std::string>&)> f = bind(con_fun, this, std::placeholders::_1);
        m_console.reg_cmdN(name, f, help_txt);
    }
}