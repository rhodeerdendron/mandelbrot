#include "rendertarget.hpp"

#include <memory>
#include <iostream>
#include <utility>

#include "program.hpp"
#include "vertex-array.hpp"


namespace {

static const float s_quad_vertices[] =
{
    // texcoord
    0.0f,  1.0f,
    0.0f,  0.0f,
    1.0f,  0.0f,

    0.0f,  1.0f,
    1.0f,  0.0f,
    1.0f,  1.0f
};

static std::unique_ptr<Program> s_texture_program = nullptr;
static std::unique_ptr<VertexArray> s_vao = nullptr;

static GLint s_tex_pos_unif = -1, s_tex_size_unif = -1, s_tex_z_unif = -1;
static GLint s_screen_size_unif = -1;

} // anonymous namespace


static inline void setup_program()
{
    if (s_texture_program) return; // already set up

    // create texture draw program
    s_texture_program = std::make_unique<Program>(
        std::filesystem::path{"shaders/texture.vert"},
        std::filesystem::path{"shaders/texture.frag"});
    // link tex sampler2D to slot 0
    glUniform1i(s_texture_program->get_uniform("tex"), 0);

    // find unifs for tex transform data
    s_tex_pos_unif = s_texture_program->get_uniform("tex_position");
    s_tex_size_unif = s_texture_program->get_uniform("tex_size");
    s_tex_z_unif = s_texture_program->get_uniform("tex_z");
    s_screen_size_unif = s_texture_program->get_uniform("screen_size");

    // initialize quad vao and vbo
    s_vao = std::make_unique<VertexArray>();
    s_vao->add_vertex_buffer(2*sizeof(float), 0);

    auto& vbo = s_vao->get_buffer(0);
    vbo.add_attrib(2, GL_FLOAT); // v_texcoord
    vbo.bind_data((void*)s_quad_vertices, 6, GL_STATIC_DRAW);
}


static inline GLuint generateFBO(void)
{
    GLuint fbo = 0;
    glGenFramebuffers(1, &fbo);
    return fbo;
}
// this is the constructor outside classes (other than Screen) will use
RenderTarget::RenderTarget(int width, int height) :
    RenderTarget(width, height, generateFBO()) {}

// this is the *actual* constructor
RenderTarget::RenderTarget(int width, int height, GLuint fbo) :
    m_fbo(fbo),
    m_width(width), m_height(height),
    m_color_texture(GL_RGB),
    m_depth_stencil_texture(GL_DEPTH24_STENCIL8)
{
    setup_program();

    use();

    // set up framebuffer
    // color texture
    m_color_texture.use();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_color_texture.id(), 0);
    // depth/stencil texture
    m_depth_stencil_texture.use();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_depth_stencil_texture.id(), 0);

    // trigger resize() to create texture attachments
    m_width = -1; m_height = -1;
    resize(width, height);
}

RenderTarget::~RenderTarget(void)
{
    // if this isn't the default framebuffer (fbo 0), delete
    if (m_fbo != 0)
        glDeleteFramebuffers(1, &m_fbo);
}

RenderTarget::RenderTarget(RenderTarget&& rhs) :
    m_width(rhs.m_width), m_height(rhs.m_height),
    m_fbo(std::exchange(rhs.m_fbo, 0)),
    m_color_texture(std::move(rhs.m_color_texture)),
    m_depth_stencil_texture(std::move(rhs.m_depth_stencil_texture))
{}
RenderTarget& RenderTarget::operator=(RenderTarget&& rhs)
{
    this->~RenderTarget();
    m_width = rhs.m_width;
    m_height = rhs.m_height;
    m_fbo = std::exchange(rhs.m_fbo, 0);
    m_color_texture = std::move(rhs.m_color_texture);
    m_depth_stencil_texture = std::move(rhs.m_depth_stencil_texture);
    return *this;
}

void RenderTarget::resize(int width, int height)
{
    // do nothing if not resizing
    if (m_width == width && m_height == height) return;
    // do nothing if sizes aren't positive
    if (width <= 0 || height <= 0)
    {
        std::cerr << "RenderTarget: cannot resize to given w: "
            << width << " h: " << height << std::endl;
        return;
    }

    // resize framebuffer
    m_width = width;
    m_height = height;
    use();

    m_color_texture.set_pixels(
        width, height,
        GL_RGB, GL_UNSIGNED_BYTE,
        NULL);
    m_depth_stencil_texture.set_pixels(
        width, height,
        GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8,
        NULL);

    // check framebuffer is OK
    GLenum complete = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (complete != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "RenderTarget: framebuffer is not complete!\nreason: ";
        switch(complete)
        {
            case GL_FRAMEBUFFER_UNDEFINED:
                std::cerr << "undefined framebuffer";
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                std::cerr << "incomplete attachment";
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                std::cerr << "missing attachment";
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
                std::cerr << "incomplete draw buffer";
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
                std::cerr << "incomplete read buffer";
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
                std::cerr << "invalid or mismatched multisample settings";
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
                std::cerr << "invalid or mismatched layer settings";
                break;
            default:
                std::cerr << "unknown error";
                break;
        }
        std::cerr << std::endl;
        exit(5);
    }
}

void RenderTarget::use(void)
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, m_width, m_height);
}

void RenderTarget::clear(void)
{
    use();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}


void RenderTarget::render_texture(
    const Texture& texture,
    int positionx, int positiony,
    int sizex, int sizey,
    float z)
{
    this->use();
    s_texture_program->use();

    glEnable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glActiveTexture(GL_TEXTURE0);
    texture.use();

    // set tex transform data
    glUniform2i(s_tex_pos_unif, positionx, positiony);
    glUniform2i(s_tex_size_unif, sizex, sizey);
    glUniform1f(s_tex_z_unif, z);
    glUniform2i(s_screen_size_unif, m_width, m_height);

    // render!
    s_vao->use();
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

