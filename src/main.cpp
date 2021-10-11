#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL.h>
#include <bx/math.h>
#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include "surfaceComponent.hpp"
#include "renderer.hpp"
#include "batchRenderer.hpp"

#include <chrono>
#include <thread>

// FPS regulator
class FrameCap {
public:
  using Clock = std::chrono::steady_clock;

  explicit FrameCap(const int fps)
    : interval{Clock::duration{Clock::duration::period::den / fps}},
      start{Clock::now()} {}

  ~FrameCap() {
    std::this_thread::sleep_until(start + interval);
  }

private:
  const Clock::duration interval;
  const Clock::time_point start;
};

int main () {
    // Init SDL
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window * myWindow = SDL_CreateWindow(
                                             "My App",
                                             SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                             800, 600,
                                             SDL_WINDOW_SHOWN
                                             );
    
    bgfx::PlatformData pd;
    
    // Get platform data
    SDL_SysWMinfo wmi;
    SDL_VERSION(&wmi.version);
    SDL_GetWindowWMInfo(myWindow, &wmi);
    
    #if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
        pd.ndt = wmi.info.x11.display;
        pd.nwh = (void*)(uintptr_t)wmi.info.x11.window;
    #elif BX_PLATFORM_OSX
        pd.ndt = NULL;
        pd.nwh = wmi.info.cocoa.window;
    #elif BX_PLATFORM_WINDOWS
        pd.ndt = NULL;
        pd.nwh = wmi.info.win.window;
    #elif BX_PLATFORM_STEAMLINK
        pd.ndt = wmi.info.vivante.display;
        pd.nwh = wmi.info.vivante.window;
    #endif // BX_PLATFORM_
        pd.context = NULL;
        pd.backBuffer = NULL;
        pd.backBufferDS = NULL;
        bgfx::setPlatformData(pd);
    
    // Init BGFX
    bgfx::renderFrame();
    bgfx::Init bgfx_init;
    bgfx_init.type = bgfx::RendererType::Count; // auto choose renderer
    bgfx_init.resolution.width = 800;
    bgfx_init.resolution.height = 600;
    bgfx_init.resolution.reset = BGFX_RESET_VSYNC | BGFX_RESET_HIDPI;
    bgfx_init.platformData = pd;
    bgfx::init(bgfx_init);
    
    // Enable debug text.
    bgfx::setDebug(BGFX_DEBUG_TEXT /*| BGFX_DEBUG_STATS*/);

    // Set view 0 clear state.
    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030ff, 1.0f, 0);
    
    bgfx::setViewRect(0, 0, 0, bgfx_init.resolution.width, bgfx_init.resolution.height);

    // Make my cube
    Engine::Surface::Mesh myObject;

    myObject.vertices = {
        {-1.0f,  1.0f,  1.0f, 0xFFFFFFFF },
        { 1.0f,  1.0f,  1.0f, 0xFFFFFFFF },
        {-1.0f, -1.0f,  1.0f, 0xFFFFFFFF },
        { 1.0f, -1.0f,  1.0f, 0xFFFFFFFF },

        {-1.0f,  1.0f, -1.0f, 0xFFFFFFFF },
        { 1.0f,  1.0f, -1.0f, 0xFFFFFFFF },
        {-1.0f, -1.0f, -1.0f, 0xFFFFFFFF },
        { 1.0f, -1.0f, -1.0f, 0xFFFFFFFF },
    };

    myObject.vertexIndices = {
        0, 2, 1,
        1, 2, 3,
        4, 5, 6,
        5, 7, 6,
        0, 4, 2,
        4, 6, 2,
        1, 3, 5,
        5, 3, 7,
        0, 1, 4,
        4, 1, 5,
        2, 6, 3,
        6, 7, 3,
    };
    
    bx::Vec3 cameraPos(5, 5, -2.5);
    bx::Quaternion cameraRot(-0.4254518, 0.4254518, -0.237339, 0.762661);
    
    bx::Vec3 up(0, 1, 0);
    bx::Vec3 forward(0, 0, 1);
    
    Engine::Renderer::BxBgfxRenderer renderer;
    
    bool exit = false;
    while (!exit) {
        FrameCap sync {60};
        
        // SDL event
        SDL_Event e;
        
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_QUIT:
                    exit = true;
                    break;
                    
                default: break;
            }
        }
        
        bgfx::touch(0); // Clear view
        
        bx::Vec3 at = bx::add(cameraPos, bx::mul(forward, cameraRot));
        renderer.setViewMtx(cameraPos, at, bx::mul(up, cameraRot));
        
        renderer.setProjMtx(90, 1, 0.01f, 1000.0f);
        renderer.setViewport(0, 0, 800, 600); // Same as window size
        renderer.prepare();
        
        // Add things to render
        renderer.add(myObject);
        
        // render the scene
        renderer.present();
        
        bgfx::frame(); // Present render
    }
    
    bgfx::shutdown();
    SDL_Quit();

    return 0;
}
