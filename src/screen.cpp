#include "screen.hpp"

#include <iostream>


// error checking for SDL calls
#define checkSDLError(val) _checkSDLError( (val), #val, __FILE__, __LINE__ )
void _checkSDLError(
    int const code,
    char const *const func,
    const char *const file,
    int const line)
{
    if (code)
    {
        std::cerr << "\nGLEW Error: " << SDL_GetError()
            << "\n...at " << file << ":" << line << " '" << func << "'"
            << std::endl;
        exit(-1);
    }
}

// error checking for GLEW calls
#define checkGLEWError(val) _checkGLEWError( (val), #val, __FILE__, __LINE__ )
void _checkGLEWError(
    GLenum const code,
    char const *const func,
    const char *const file,
    int const line)
{
    if (code != GLEW_OK)
    {
        std::cerr << "\nGLEW Error: " << glewGetErrorString(code)
            << "\n...at " << file << ":" << line << " '" << func << "'"
            << std::endl;
        exit(-1);
    }
}


namespace {
static std::size_t sg_SDL_INIT_COUNT = 0;
}

Screen::Screen(uint32_t width, uint32_t height, const char* title) :
    m_width(width), m_height(height)
{
    if (sg_SDL_INIT_COUNT++ == 0)
    {
        // init SDL2 and SDL_ttf
        checkSDLError(SDL_Init( // 0 on error
            SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_EVENTS));

        // use opengl 3.3-core
        SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
        SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 3 );
        SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
    }

    // create the window
    mp_window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        m_width, m_height,
        /*SDL_WINDOW_RESIZABLE |*/ SDL_WINDOW_OPENGL);
    checkSDLError(mp_window == NULL);
    m_windowID = SDL_GetWindowID(mp_window);

    // create the opengl context
    mp_GLContext = SDL_GL_CreateContext(mp_window);
    checkSDLError(mp_GLContext == NULL);

    // init GLEW
    glewExperimental = GL_TRUE;
    checkGLEWError(glewInit());

    // enable Vsync
    if (SDL_GL_SetSwapInterval(1) < 0)
        printf("warning: could not enable vsync\nSDL error: %s\n", SDL_GetError());

    // // disable Vsync
    // if (SDL_GL_SetSwapInterval(0) < 0)
    //     printf("warning: could not disable vsync\nSDL error: %s\n", SDL_GetError());

    // set up rendertarget
    m_rendertarget = RenderTarget(width, height, 0);

    // set up state flags
    m_flags = 0
        | static_cast<uint32_t>(WindowFlag::Shown)
        | static_cast<uint32_t>(WindowFlag::HasMouseFocus)
        | static_cast<uint32_t>(WindowFlag::HasKeyboardFocus);
}

Screen::~Screen()
{
    if (mp_GLContext) SDL_GL_DeleteContext(mp_GLContext);
    mp_GLContext = nullptr;
    if (mp_window) SDL_DestroyWindow(mp_window);
    mp_window = nullptr;

    // quit SDL2 subsystems
    if (--sg_SDL_INIT_COUNT == 0)
        SDL_Quit();
}


Screen::Screen(Screen&& rhs) :
    m_width(rhs.m_width), m_height(rhs.m_height),
    m_windowID(std::exchange(rhs.m_windowID, -1)),
    mp_window(std::exchange(rhs.mp_window, nullptr)),
    mp_GLContext(std::exchange(rhs.mp_GLContext, nullptr)),
    m_flags(rhs.m_flags)
{}

Screen& Screen::operator=(Screen&& rhs)
{
    if (mp_window) SDL_DestroyWindow(mp_window);
    if (mp_GLContext) SDL_GL_DeleteContext(mp_GLContext);

    m_width = rhs.m_width, m_height = rhs.m_height;
    m_windowID = std::exchange(rhs.m_windowID, -1);
    mp_window = std::exchange(rhs.mp_window, nullptr);
    mp_GLContext = std::exchange(rhs.mp_GLContext, nullptr);
    m_flags = rhs.m_flags;

    return *this;
}


void Screen::resize(uint32_t width, uint32_t height)
{
    if (m_windowID == (Uint32)(-1)) return;
    
    SDL_SetWindowSize(mp_window, width, height);

    // TODO resize framebuffer

    m_width  = width;
    m_height = height;
}


void Screen::grab_focus(void)
{
    if (m_windowID == (Uint32)(-1)) return;

    if (!getFlag(WindowFlag::Shown))
        SDL_ShowWindow(mp_window);
    SDL_RaiseWindow(mp_window);
}


void Screen::flip(void)
{
    if (m_windowID == (Uint32)(-1)) return;

    SDL_GL_SwapWindow(mp_window);
}


bool Screen::process_event(SDL_Event& e)
{
    // quit event
    if (e.type == SDL_QUIT)
    {
        printf("screen: quit event received\n");
        return false;
    }

    // handle window events
    else if (e.type == SDL_WINDOWEVENT)
    {
        // is this our event?
        if (e.window.windowID != m_windowID)
        {
            printf("screen: event not for this window (for %d, we are %d)\n", e.window.windowID, m_windowID);
            return true;
        }

        switch(e.window.event)
        {
            // mouse focus
            case SDL_WINDOWEVENT_ENTER:
                setFlag(WindowFlag::HasMouseFocus);
                break;
            case SDL_WINDOWEVENT_LEAVE:
                clearFlag(WindowFlag::HasMouseFocus);
                break;

            // keyboard focus
            case SDL_WINDOWEVENT_FOCUS_GAINED:
                setFlag(WindowFlag::HasKeyboardFocus);
                break;
            case SDL_WINDOWEVENT_FOCUS_LOST:
                clearFlag(WindowFlag::HasKeyboardFocus);
                break;

            // window has been hidden/exposed
            case SDL_WINDOWEVENT_SHOWN:
                setFlag(WindowFlag::Shown);
                [[fallthrough]];
            case SDL_WINDOWEVENT_EXPOSED:
                SDL_GL_SwapWindow(mp_window);
                break;
            case SDL_WINDOWEVENT_HIDDEN:
                clearFlag(WindowFlag::Shown);
                break;

            // window resize
            case SDL_WINDOWEVENT_SIZE_CHANGED:
                resize(e.window.data1, e.window.data2);
                break;

            // window maximize/minimize
            case SDL_WINDOWEVENT_MINIMIZED:
                setFlag(WindowFlag::Minimized);
                break;
            case SDL_WINDOWEVENT_MAXIMIZED:
            case SDL_WINDOWEVENT_RESTORED:
                clearFlag(WindowFlag::Minimized);
                break;

            // window close, but not quitting SDL (multiple windows?)
            case SDL_WINDOWEVENT_CLOSE:
                clearFlag(WindowFlag::Shown);
                setFlag(WindowFlag::Closed);
                SDL_HideWindow(mp_window);
                break;
        }
    }

    return true;
}

