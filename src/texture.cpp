#include "texture.hpp"

#include <iostream>
#include <utility>


static inline GLuint generate_texture_id(void)
{
    GLuint id;
    glGenTextures(1, &id);
    return id;
}

Texture::Texture(GLint internal_format) :
    m_id(generate_texture_id()),
    m_internal_format(internal_format)
{ use(); }

Texture::~Texture(void)
{
    if (m_id != 0)
        glDeleteTextures(1, &m_id);
}


Texture::Texture(Texture&& rhs) :
    m_id(std::exchange(rhs.m_id, 0)),
    m_internal_format(rhs.m_internal_format),
    m_width(rhs.m_width), m_height(rhs.m_height)
{}
Texture& Texture::operator=(Texture&& rhs)
{
    this->~Texture();
    m_id = std::exchange(rhs.m_id, 0);
    m_internal_format = rhs.m_internal_format;
    m_width = rhs.m_width;
    m_height = rhs.m_height;
    return *this;
}


void Texture::use(void) const
{
    glBindTexture(GL_TEXTURE_2D, m_id);
}

void Texture::generate_mipmap(void)
{
    use();
    glGenerateMipmap(GL_TEXTURE_2D);
}

void Texture::set_pixels(
    uint32_t width, uint32_t height,
    GLenum pixels_format, GLenum pixels_datatype,
    const void* data)
{
    if (m_id == 0)
    {
        std::cerr << "Texture: no device texture generated"
            " (default ctor used?)" << std::endl;
        return;
    }

    m_width = width;
    m_height = height;

    use();
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        m_internal_format,
        m_width, m_height,
        0,
        pixels_format, pixels_datatype,
        data);
}

