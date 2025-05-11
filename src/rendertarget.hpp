#ifndef RENDERTARGETH
#define RENDERTARGETH

#include <GL/glew.h>
#include <GL/glu.h>

#include "texture.hpp"


class RenderTarget
{
    friend class Screen;
    // the Screen class will spawn a special RenderTarget2D that has its
    // m_framebufferID == 0 (so that rendering to it draws to the screen)
    // this will be called from the regular constructor with fbo != 0, and
    // from Screen with fbo == 0
    RenderTarget(int width, int height, GLuint fbo);

public:
    // this is the constructor outside classes (other than Screen) will use
    RenderTarget(void) = default;
    RenderTarget(int width, int height);
    ~RenderTarget(void);

    RenderTarget(const RenderTarget&) = delete;
    RenderTarget& operator=(const RenderTarget&) = delete;

    RenderTarget(RenderTarget&& rhs);
    RenderTarget& operator=(RenderTarget&& rhs);

public:
    void resize(int width, int height);
    inline int width (void) const { return m_width; }
    inline int height(void) const { return m_height; }

    inline GLuint fbo(void) const { return m_fbo; }

    inline       Texture& color_texture(void)       { return m_color_texture; }
    inline const Texture& color_texture(void) const { return m_color_texture; }
    inline       Texture& depth_stencil_texture(void)       { return m_depth_stencil_texture; }
    inline const Texture& depth_stencil_texture(void) const { return m_depth_stencil_texture; }

    void use(void);
    void clear(void);

    // render the given texture(s) onto this RenderTarget
    void render_texture(
        const Texture& texture,
        int x, int y, // position, in screen-space (x=[0,w) and y=[0,h))
        int width, int height, // size, in screen-space
        float z); // z, in clip-space (z=[-1,1])

private:
    int m_width = 0, m_height = 0;
    GLuint m_fbo = 0;
    Texture m_color_texture, m_depth_stencil_texture;
};


#endif //RENDERTARGETH

