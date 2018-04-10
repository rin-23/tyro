#include "TyroApp.h"
#include "RAEnginePrerequisites.h"
#include "Wm5APoint.h"
#include "Wm5Vector3.h"
#include "RAVisibleSet.h"
#include "RAES2StandardMesh.h"
#include <functional>
#include <igl/readOBJ.h>
#include <igl/per_vertex_normals.h>
#include "TyroIGLMesh.h"
#include <igl/unproject_onto_mesh.h>

using namespace Wm5;
using namespace std;

namespace tyro
{   
    namespace
    {
        void console_load_obj(App* app, const std::vector<std::string> & args) 
        {
            RA_LOG_INFO("Loading obj model");

            if (args.size() == 0)
            {   
                app->load_hiroki();
                return;
            }
        }
    }

    App::App()
    :
    m_tyro_window(nullptr),
    m_gl_rend(nullptr),
    m_camera(nullptr),
    mouse_is_down(false),
    gesture_state(0),
    show_console(false)
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

        //setup window
        m_tyro_window = new Window();
        m_tyro_window->Init();
                
        //setup renderer
        m_gl_rend = new ES2Renderer(m_tyro_window->GetGLContext());
        m_gl_rend->SetClearColor(Vector4f(0.0, 153.0/255.0, 153.0/255.0, 1.0));
                
        //setup objects to draw
        //ES2SphereSPtr object = ES2Sphereorner::Create(100, 100, 100);
        //ES2BoxSPtr object = ES2Box::Create(10);
        //object->Update(true);
        
        
        //VisibleSet* vis_set = new VisibleSet();
        //vis_set->Insert(object.get());
        
        //setup camera
        /*
        APoint world_center = object->WorldBoundBox.GetCenter();
        float radius = std::abs(object->WorldBoundBox.GetRadius()*2);
        float aspect = 1.0;
        */
        int v_width, v_height;
        m_tyro_window->GetGLContext()->getFramebufferSize(&v_width, &v_height);
        Vector4i viewport(0, 0, v_width, v_height);
        m_camera = new iOSCamera(APoint(0,0,0), 1.0, 1.0, 2, viewport, true);
                
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

        register_console_function("load_obj", console_load_obj, "");
       

        // Loop until the user closes the window
        while (!m_tyro_window->ShouldClose())
        {
            // Render here 
            VisibleSet vis_set;
            for (auto object_sptr : object_list) {
                vis_set.Insert(object_sptr.get());
            }
        
            m_gl_rend->RenderVisibleSet(&vis_set, m_camera);
            
            // Draw console
            if (show_console) 
            {
                glUseProgram(0);
                m_console.display(2);
            }

            // Poll for and process events
            m_tyro_window->GetGLContext()->swapBuffers();
            m_tyro_window->ProcessUserEvents();
        }

        //free memmory
        //object = nullptr;
        //delete vis_set;
        
	    return 0;
    }

    void App::load_hiroki() 
    {   
        // load OBJ file
        //std::string path = std::string("/home/rinat/tmp/OldmanFramesOBJ/85_fixed.obj");
        //std::string path = std::string("/home/rinat/Workspace/Tyro/libigl/tutorial/shared/armadillo.obj");
        //std::string path = std::string("/home/rinat/Google Drive/StopMotionProject/Claymation/data/uw/FramesOBJ/Lower/bottom-face_mesh_000001.obj");
        std::string path = std::string("/home/rinat/Google Drive/StopMotionProject/Claymation/data/hello/FewSelected/frame.0000.obj");
        
        Eigen::MatrixXd V, N;
        Eigen::MatrixXi F;
        igl::readOBJ(path, V, F);
        int num_face = F.rows();
        igl::per_vertex_normals(V,F,N);        

        //remove previous objects
        object_list.clear();
        
        //create renderable for mesh
        igl_mesh = IGLMesh::Create(V, F, N);
        igl_mesh->Update(true);
        object_list.push_back(igl_mesh);

        //create renderable for mesh wireframe
        igl_mesh_wire = IGLMeshWireframe::Create(V, F);
        igl_mesh_wire->Update(true);
        object_list.push_back(igl_mesh_wire);

        //setup camera
        APoint world_center = igl_mesh->WorldBoundBox.GetCenter();
        float radius = std::abs(igl_mesh->WorldBoundBox.GetRadius()*2);
        float aspect = 1.0;
        int v_width, v_height;
        m_tyro_window->GetGLContext()->getFramebufferSize(&v_width, &v_height);
        Vector4i viewport(0, 0, v_width, v_height);
        
        if (m_camera)
            delete m_camera;
        
        m_camera = new iOSCamera(world_center, radius, aspect, 2, viewport, true);
    }
        
    void App::mouse_down(Window& window, int button, int modifier) 
    {   
        if (object_list.size() == 0) return;
        //RA_LOG_INFO("mouse down");
        mouse_is_down = true;
   
        int fid;
        Eigen::Vector3f bc;
        // Cast a ray in the view direction starting from the mouse position
        double x = current_mouse_x;
        double y = m_camera->GetViewport()[3] - current_mouse_y;
        Eigen::Vector2f mouse_pos(x,y);
        Wm5::HMatrix modelViewMatrix = m_camera->GetViewMatrix() * igl_mesh->WorldTransform.Matrix();
        Wm5::HMatrix projectMatrix = m_camera->GetProjectionMatrix();
        Eigen::Matrix4f e1 = Eigen::Map<Eigen::Matrix4f>(modelViewMatrix.mEntry);
        Eigen::Matrix4f e2 = Eigen::Map<Eigen::Matrix4f>(projectMatrix.mEntry);
        Eigen::Vector4f e3 = Eigen::Vector4f(m_camera->GetViewport()[0],
                                             m_camera->GetViewport()[1],
                                             m_camera->GetViewport()[2],
                                             m_camera->GetViewport()[3]);

        /*
         IGL_INLINE bool unproject_onto_mesh(
        const Eigen::Vector2f& p   os,
        const Eigen::Matrix4f& model,
        const Eigen::Matrix4f& proj,
        const Eigen::Vector4f& viewport,
        const Eigen::PlainObjectBase<DerivedV> & V,
        const Eigen::PlainObjectBase<DerivedF> & F,
        int & fid,
        Eigen::PlainObjectBase<Derivedbc> & bc);
        */
        
        if (igl::unproject_onto_mesh(mouse_pos, 
                                     e1.transpose(),
                                     e2.transpose(),
                                     e3,
                                     igl_mesh->V,
                                     igl_mesh->F,
                                     fid,
                                     bc))
        {
            long c;
            bc.maxCoeff(&c);
            int vid = igl_mesh->F(fid, c);
            Eigen::RowVector3d new_c = igl_mesh->V.row(vid);
            RA_LOG_INFO("Picked face_id %i vertex_id %i", fid, vid);
            
            ES2SphereSPtr object = ES2Sphere::Create(10, 10, 0.1);
            Wm5::Transform tr;
            tr.SetTranslate(APoint(new_c(0), new_c(1), new_c(2)));
            object->LocalTransform = tr * object->LocalTransform;
            object->Update(true);
            object_list.push_back(object);
              
            //if(s.CV.size()==0 || (s.CV.rowwise() - new_c).rowwise().norm().minCoeff() > 0)
            //{
                //push_undo();
                //s.CV.conservativeResize(s.CV.rows()+1,3);
                // Snap to closest vertex on hit face
                //s.CV.row(s.CV.rows()-1) = new_c;
                //update();
                //return true;
            //}
        };
    }

    void App::mouse_up(Window& window, int button, int modifier) 
    {   
        if (object_list.size() == 0) return;
        RA_LOG_INFO("modifier %i", modifier);
        if (mouse_is_down) 
        {   
            gesture_state = 2;
            //m_camera->HandleOneFingerPanGesture(gesture_state, Vector2i(mouse_x, mouse_y));
        }
        mouse_is_down = false;
        gesture_state = 0;
    }
    
    void App::mouse_move(Window& window, int mouse_x, int mouse_y) 
    {   
        if (object_list.size() == 0) return;

       // RA_LOG_INFO("mouse move");
        current_mouse_x = mouse_x;
        current_mouse_y = mouse_y;
        if (mouse_is_down) 
        {   
            m_camera->HandleOneFingerPanGesture(gesture_state, Vector2i(mouse_x, mouse_y));
            gesture_state = 1;
        }
    }
    
    void App::window_resize(Window& window, unsigned int w, unsigned int h)
    {
        //RA_LOG_INFO("window resized")
        //float  aspect = m_gl_rend->GetViewWidth()/ (float) m_gl_rend->GetViewHeight();
        //Vector4i viewport(0, 0, m_gl_rend->GetViewWidth(), m_gl_rend->GetViewHeight());
        //m_camera->SetAspect(aspect);
        //m_camera->SetViewport(viewport);
    }

    void App::key_pressed(Window& window, unsigned int key, int modifiers) 
    {   
        //RA_LOG_INFO("Key pressed %c", key);
        
        if (key == '`') 
        {
            show_console = !show_console;
            return;
        }

        if (show_console) 
        {
           m_console.keyboard(key);
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

        if (key == TYRO_KEY_LEFT) 
            m_console.key_left();
        else if (key == TYRO_KEY_RIGHT) 
            m_console.key_right();
         else if (key == TYRO_KEY_ENTER)
            m_console.key_enter();
        else if (key == TYRO_KEY_BACKSPACE)
            m_console.key_backspace();
        else if (key == TYRO_KEY_UP)
            m_console.key_up();
        else if (key == TYRO_KEY_DOWN)
            m_console.key_down();
        else if (key == TYRO_KEY_TAB)
            m_console.key_tab();
            
    }

     void App::register_console_function(const std::string& name,
                                         const std::function<void(App*, const std::vector<std::string>&)>& con_fun,
                                         const std::string& help_txt)
    {
        std::function<void (const std::vector<std::string>&)> f = bind(con_fun, this, std::placeholders::_1);
        m_console.reg_cmdN(name, f, help_txt);
    }
}