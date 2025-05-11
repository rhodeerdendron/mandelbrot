#ifndef SCREENH
#define SCREENH

#include <stdint.h>
#include <vector>
#include <utility>
#include <memory>

#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <SDL2/SDL_opengl.h>

#include "rendertarget.hpp"


class Screen
{
public:
    Screen(uint32_t width, uint32_t height) : Screen(width, height, "OpenGL") {}
    Screen(uint32_t width, uint32_t height, const char* title);
    ~Screen();

    // not copyable
    Screen(const Screen&) = delete;
    Screen& operator=(const Screen&) = delete;

    // movable
    Screen(Screen&&);
    Screen& operator=(Screen&&);

    // flip rendertarget to screen
    void flip(void);

    bool process_event(SDL_Event& e);

    uint32_t width(void) const { return m_width; }
    uint32_t height(void) const { return m_height; }

    void set_title(const char* title) { SDL_SetWindowTitle(mp_window, title); }

    void resize(uint32_t width, uint32_t height);
    void grab_focus(void);
    void set_mouse(int x, int y) { SDL_WarpMouseInWindow(mp_window, x, y); }

          RenderTarget& get_rendertarget(void)       { return m_rendertarget; }
    const RenderTarget& get_rendertarget(void) const { return m_rendertarget; }

    SDL_Window* window(void) const { return mp_window; }
    // this isnt a ptr here because SDL_video.h : typedef void* SDL_GLContext
    SDL_GLContext GLContext(void) const { return mp_GLContext; }

private:
    uint32_t m_width, m_height;

    RenderTarget m_rendertarget;

    Uint32 m_windowID = (Uint32)(-1);  // SDL's u32 type (typedef to uint32_t)
    SDL_Window* mp_window = nullptr;
    // this isnt a ptr here because SDL_video.h : typedef void* SDL_GLContext
    SDL_GLContext mp_GLContext = nullptr;

    // flags about the screen state
    uint8_t m_flags = 0;
public:
    enum class WindowFlag : uint8_t
    {
        Shown               = (1u << 0),
        Closed              = (1u << 1),
        Minimized           = (1u << 2),
        Fullscreen          = (1u << 3),
        HasMouseFocus       = (1u << 4),
        HasKeyboardFocus    = (1u << 5),
    };
    inline bool getFlag(WindowFlag flag) const
    {
        return m_flags & static_cast<uint64_t>(flag);
    }
private:
    inline void setFlag(WindowFlag flag)
    {
        m_flags |= static_cast<uint64_t>(flag);
    }
    inline void clearFlag(WindowFlag flag)
    {
        m_flags &= ~static_cast<uint64_t>(flag);
    }
};

#endif // SCREENH
