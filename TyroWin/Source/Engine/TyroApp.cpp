#include "TyroApp.h"
#include "RAEnginePrerequisites.h"
#include "Wm5APoint.h"
#include "Wm5Vector3.h"
#include "RAVisibleSet.h"
#include "RAES2StandardMesh.h"

using namespace Wm5;

namespace tyro
{
    App::App()
    :
    m_tyro_window(nullptr),
    m_gl_rend(nullptr),
    m_camera(nullptr)
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
        RA_LOG_INFO("Setup window");
                
        //setup renderer
        m_gl_rend = new ES2Renderer(m_tyro_window->GetGLContext());
        m_gl_rend->SetClearColor(Vector4f(0.0, 1.0, 0.0, 1.0));
        RA_LOG_INFO("Setup renderer");
                
        //setup objects to draw
        //ES2SphereSPtr object = ES2Sphere::Create(100, 100, 100);
        ES2BoxSPtr object = ES2Box::Create(10);
        object->Update(true);
        VisibleSet* vis_set = new VisibleSet();
        vis_set->Insert(object.get());
        
        //setup camera
        APoint world_center = object->WorldBoundBox.GetCenter();
        float radius = std::abs(object->WorldBoundBox.GetRadius()*2);
        float aspect = 1.0;
        int v_width, v_height;
        m_tyro_window->GetGLContext()->getFramebufferSize(&v_width, &v_height);
        Vector4i viewport(0, 0, v_width, v_height);
        m_camera = new iOSCamera(world_center, radius, aspect, 2, viewport, true);
        
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

        // Loop until the user closes the window
        while (!m_tyro_window->ShouldClose())
        {
            // Render here 
            m_gl_rend->RenderVisibleSet(vis_set, m_camera);

            // Poll for and process events
            m_tyro_window->ProcessUserEvents();
        }

        //free memmory
        object = nullptr;
        delete vis_set;
        
	    return 0;
    }

    void App::mouse_down(Window& window, int button, int modifier) 
    {
        RA_LOG_INFO("mouse down");
    }

    void App::mouse_up(Window& window, int button, int modifier) 
    {
        RA_LOG_INFO("mouse up");
    }
    
    void App::mouse_move(Window& window, int mouse_x, int mouse_y) 
    {
        RA_LOG_INFO("mouse move");
    }

    void App::window_resize(Window& window, unsigned int w, unsigned int h)
    {
        RA_LOG_INFO("window resized")
        //float  aspect = m_gl_rend->GetViewWidth()/ (float) m_gl_rend->GetViewHeight();
        //Vector4i viewport(0, 0, m_gl_rend->GetViewWidth(), m_gl_rend->GetViewHeight());
        //m_camera->SetAspect(aspect);
        //m_camera->SetViewport(viewport);
    }    
}