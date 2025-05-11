#ifndef VERTEXARRAYH
#define VERTEXARRAYH

#include <vector>

#include <GL/glew.h>
#include <GL/gl.h>


class VertexBuffer
{
private:
    friend class VertexArray;
    explicit VertexBuffer(GLsizei stride, GLuint divisor);

    void use(void) const;

public:
    ~VertexBuffer(void);

    VertexBuffer(const VertexBuffer&) = delete;
    VertexBuffer& operator=(const VertexBuffer&) = delete;

    VertexBuffer(VertexBuffer&& rhs);
    VertexBuffer& operator=(VertexBuffer&& rhs);

public:
    // e.g. for ivec2, count=2 type=GL_INT
    // automatically calculates idx and offset
    void add_attrib(GLint count, GLenum type);

    void bind_data(void* data, GLsizei count, GLenum usage);

private:
    GLsizei m_stride = 0;
    GLuint m_divisor = 0;

    GLuint m_vbo = 0;
    GLuint m_working_attrib_idx = 0;
    GLuint m_working_offset = 0;
};


class VertexArray
{
public:
    VertexArray(void);
    ~VertexArray(void);

    VertexArray(const VertexArray&) = delete;
    VertexArray& operator=(const VertexArray&) = delete;

    VertexArray(VertexArray&& rhs);
    VertexArray& operator=(VertexArray&& rhs);

public:
    std::size_t add_vertex_buffer(GLsizei stride, GLuint divisor);

          VertexBuffer& get_buffer(std::size_t i);
    const VertexBuffer& get_buffer(std::size_t i) const;

    void use(void) const;

private:
    GLuint m_vao = 0;
    std::vector<VertexBuffer> m_buffers;
};


#endif // VERTEXARRAYH
