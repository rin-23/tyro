#include "TyroWindow.h"
#include "RAES2Renderer.h"
#include "RAiOSCamera.h"

namespace tyro
{
    class App 
    {
    public:
        App();
        
        ~App();
        
        int Launch();

    private:
        Window* m_tyro_window;
        ES2Renderer* m_gl_rend;
        iOSCamera* m_camera;

        void mouse_down(Window& window, int button, int modifier);
        void mouse_up(Window& window, int button, int modifier);
        void mouse_move(Window& window, int mouse_x, int mouse_y);
        void window_resize(Window& window, unsigned int w, unsigned int h);    
    };
}