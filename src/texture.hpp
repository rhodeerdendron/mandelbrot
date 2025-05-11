#ifndef TEXTUREH
#define TEXTUREH

#include <stdint.h>

#include <GL/glew.h>
#include <GL/gl.h>


class Texture
{
public:
    Texture(void) = default;
    Texture(GLint internal_format);
    ~Texture(void);

    Texture(const Texture& rhs) = delete;
    Texture& operator=(const Texture& rhs) = delete;

    Texture(Texture&& rhs);
    Texture& operator=(Texture&& rhs);

public:
    void set_pixels(
        uint32_t width, uint32_t height,
        GLenum pixels_format, /* e.x. GL_RGB */
        GLenum pixels_datatype, /* e.x. GL_UNSIGNED_BYTE */
        const void* data);

    // glBindTexture
    void use(void) const;

    // glGenerateMipmap
    void generate_mipmap(void);

    GLuint get_id(void) const { return m_id; }
    GLint get_internal_format(void) const { return m_internal_format; }
    uint32_t get_width(void) const { return m_width; }
    uint32_t get_height(void) const { return m_height; }

private:
    GLuint m_id = 0;
    GLint m_internal_format = 0; // e.x. GL_RGB
    uint32_t m_width = 0, m_height = 0;
};

#endif // TEXTUREH
