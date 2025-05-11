#include <iostream>
#include <stdint.h>
#include <stdlib.h>
#include <string_view>

#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <GL/gl.h>

#include "screen.hpp"
#include "texture.hpp"
#include "text.hpp"
#include "rendertarget.hpp"
#include "vertex-array.hpp"



constexpr static uint32_t MAX_DEPTH = 1024;

const float quad_vertices[] =
{
    // position
    -1.0f,  1.0f,
    -1.0f, -1.0f,
     1.0f, -1.0f,

    -1.0f,  1.0f,
     1.0f, -1.0f,
     1.0f,  1.0f
};


// arcane mythic runes from the opengl docs
void GLAPIENTRY MessageCallback(
    [[maybe_unused]] GLenum source,
                     GLenum type,
    [[maybe_unused]] GLuint id,
                     GLenum severity,
    [[maybe_unused]] GLsizei length,
                     const GLchar* message,
    [[maybe_unused]] const void* userParam)
{
    if (type != GL_DEBUG_TYPE_ERROR) return;

    fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
        ((type == GL_DEBUG_TYPE_ERROR)? "** GL ERROR **" : ""),
        type, severity, message);
    fflush(stderr);
}

int main()
{
    Screen screen(1280, 720, "Mandelbrot");

    // enable debug output
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(MessageCallback, 0);

    // current exponent and threshhold information
    double exponent = 2.0;
    double threshhold = 2.0;

    // current view settings
    double centerx = 0.0, centery = 0.0;
    double zoom = 0.4;

    Program prog_mandelbrot(
        std::filesystem::path{"shaders/mandelbrot.vert"},
        std::filesystem::path{"shaders/mandelbrot.frag"});
    prog_mandelbrot.use();

    // only need to set aspect and max depth once up-front
    glUniform1f(
        prog_mandelbrot.get_uniform("aspect"),
        (float)screen.width() / screen.height());
    glUniform1ui(
        prog_mandelbrot.get_uniform("max_steps"),
        MAX_DEPTH);

    GLint unif_exponent   = prog_mandelbrot.get_uniform("expon");
    GLint unif_threshhold = prog_mandelbrot.get_uniform("thresh");
    GLint unif_center     = prog_mandelbrot.get_uniform("center");
    GLint unif_zoom       = prog_mandelbrot.get_uniform("zoom");

    // for rendering mandelbrot program
    VertexArray vao_mandelbrot;
    vao_mandelbrot.add_vertex_buffer(2*sizeof(float), 0);
    // upload quad
    auto& vbo = vao_mandelbrot.get_buffer(0);
    vbo.add_attrib(2, GL_FLOAT); // vec2 v_position
    vbo.bind_data((void*)quad_vertices, 6, GL_STATIC_DRAW);

    // target to render the fractal to
    RenderTarget target_mandelbrot(screen.width(), screen.height());

    // for writing debug texts
    Font font("NotoSansMono-Regular.ttf", 16);
    char strbuf[64] {0};

    // fetch keyboard state pointer
    const Uint8* const keyboard = SDL_GetKeyboardState(NULL);

    // main loop
    SDL_Event e;
    while (true)
    {
        // handle events
        while (SDL_PollEvent(&e))
            if (!screen.process_event(e))
                goto quit;
            else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)
                goto quit;

        // handle keyboard (arbitrary sensitivities)
        const double lshift = keyboard[SDL_SCANCODE_LSHIFT]? 5.0 : 1.0;
        const double deltacenter = lshift * 0.05 / zoom;
        const double deltazoom =   lshift * 0.05;
        const double deltaexp =    lshift * 0.005;
        const double deltathresh = lshift * 0.05;
        if (keyboard[SDL_SCANCODE_S])  // -imag
            centery -= deltacenter;
        if (keyboard[SDL_SCANCODE_W])  // +imag
            centery += deltacenter;
        if (keyboard[SDL_SCANCODE_A])  // -real
            centerx -= deltacenter;
        if (keyboard[SDL_SCANCODE_D])  // +real
            centerx += deltacenter;
        if (keyboard[SDL_SCANCODE_Q])  // -zoom
            zoom -= zoom * deltazoom;
        if (keyboard[SDL_SCANCODE_E])  // +zoom
            zoom += zoom * deltazoom;
        if (keyboard[SDL_SCANCODE_LEFTBRACKET])  // -exp
            exponent -= deltaexp;
        if (keyboard[SDL_SCANCODE_RIGHTBRACKET])  // +exp
            exponent += deltaexp;
        if (keyboard[SDL_SCANCODE_MINUS])  // -thresh
            threshhold -= deltathresh;
        if (keyboard[SDL_SCANCODE_EQUALS])  // +thresh
            threshhold += deltathresh;
        if (keyboard[SDL_SCANCODE_R])  // reset view
        {
            centerx = 0.0;
            centery = 0.0;
            zoom = 0.4;
        }

        // update uniforms
        prog_mandelbrot.use();
        glUniform1f(unif_exponent, exponent);
        glUniform1f(unif_threshhold, threshhold);
        glUniform2f(unif_center, centerx, centery);
        glUniform1f(unif_zoom, zoom);

        // does fractal rendertarget need resizing?
        // FIXME

        // draw fractal
        target_mandelbrot.clear(); // also calls .use()
        prog_mandelbrot.use();
        vao_mandelbrot.use();
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // blit fractal to screen
        screen.get_rendertarget().clear(); // also calls .use()
        screen.get_rendertarget().render_texture(
            target_mandelbrot.color_texture(),
            0, 0,
            screen.width(), screen.height(),
            0.0f);

        // pos+zoom string
        {
            // draw text
            snprintf(strbuf, sizeof(strbuf),
                "pos: %+.5f%+.5fi zoom: %6fx",
                centerx, centery, zoom);
            std::string_view sv{strbuf, sizeof(strbuf)};
            Texture strtex = font.render_text_fast_bitmap(sv, GL_RED);
            strtex.use();
            strtex.generate_mipmap();

            // map red channel to white
            const GLint swizzle_mask[] = {GL_RED, GL_RED, GL_RED, GL_ONE};
            glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzle_mask);

            // blit texture to screen, top left
            screen.get_rendertarget().render_texture(
                strtex,
                screen.width() - strtex.width(), 0,
                strtex.width(), strtex.height(),
                -0.5f);
        }

        // exponent+threshhold string
        {
            // draw text
            snprintf(strbuf, sizeof(strbuf),
                "exp: %+2f thresh: %2f",
                exponent, threshhold);
            std::string_view sv{strbuf, sizeof(strbuf)};
            Texture strtex = font.render_text_fast_bitmap(sv, GL_RED);
            strtex.use();
            strtex.generate_mipmap();

            // map red channel to white
            const GLint swizzle_mask[] = {GL_RED, GL_RED, GL_RED, GL_ONE};
            glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzle_mask);

            // blit texture to screen, top left
            screen.get_rendertarget().render_texture(
                strtex,
                screen.width() - strtex.width(), 22,
                strtex.width(), strtex.height(),
                -0.5f);
        }

        // display
        screen.flip();
    }

quit:
    return 0;
}

