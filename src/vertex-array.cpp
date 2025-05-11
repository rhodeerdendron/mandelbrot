#include "vertex-array.hpp"

#include <iostream>
#include <utility>


static inline GLuint generate_vertex_buffer_id(void)
{
    GLuint id = 0;
    glGenBuffers(1, &id);
    return id;
}

static inline GLuint generate_vertex_array_id(void)
{
    GLuint id = 0;
    glGenVertexArrays(1, &id);
    return id;
}

// since this will be only ever called from VertexArray, we can assume a VAO
// is bound in gl
VertexBuffer::VertexBuffer(GLsizei stride, GLuint divisor) :
    m_stride(stride), m_divisor(divisor),
    m_vbo(generate_vertex_buffer_id())
{ use(); }

VertexBuffer::~VertexBuffer(void)
{
    glDeleteBuffers(1, &m_vbo);
}

VertexBuffer::VertexBuffer(VertexBuffer&& rhs) :
    m_stride(rhs.m_stride), m_divisor(rhs.m_divisor),
    m_vbo(std::exchange(rhs.m_vbo, 0)),
    m_working_attrib_idx(rhs.m_working_attrib_idx),
    m_working_offset(rhs.m_working_offset)
{}

VertexBuffer& VertexBuffer::operator=(VertexBuffer&& rhs)
{
    this->~VertexBuffer();
    m_stride = rhs.m_stride;
    m_divisor = rhs.m_divisor;
    m_vbo = std::exchange(rhs.m_vbo, 0);
    m_working_attrib_idx = rhs.m_working_attrib_idx;
    m_working_offset = rhs.m_working_offset;
    return *this;
}

// again, this will only ever be called from VertexArray (or from VertexBuffer
// in a method called from VertexArray), so we can assume a VAO is bound in gl
void VertexBuffer::use(void) const
{ glBindBuffer(GL_ARRAY_BUFFER, m_vbo); }


void VertexBuffer::add_attrib(GLint count, GLenum type)
{
    use();

    // translate type to size
    GLuint size = 0;
    switch (type)
    {
        case GL_BYTE:          [[fallthrough]];
        case GL_UNSIGNED_BYTE:
            size = 1;
            break;

        case GL_SHORT:          [[fallthrough]];
        case GL_UNSIGNED_SHORT: [[fallthrough]];
        case GL_HALF_FLOAT:
            size = 2;
            break;

        case GL_INT:                          [[fallthrough]];
        case GL_UNSIGNED_INT:                 [[fallthrough]];
        case GL_FLOAT:                        [[fallthrough]];
        case GL_FIXED:                        [[fallthrough]];
        case GL_INT_2_10_10_10_REV:           [[fallthrough]];
        case GL_UNSIGNED_INT_2_10_10_10_REV:  [[fallthrough]];
        case GL_UNSIGNED_INT_10F_11F_11F_REV:
            size = 4;
            break;

        case GL_DOUBLE:
            size = 8;
            break;

        default:
            std::cerr << "unknown type enum " << type << std::endl;
            return;
    }

    // translate type to function
    switch (type)
    {
        case GL_BYTE:           [[fallthrough]];
        case GL_UNSIGNED_BYTE:  [[fallthrough]];
        case GL_SHORT:          [[fallthrough]];
        case GL_UNSIGNED_SHORT: [[fallthrough]];
        case GL_INT:            [[fallthrough]];
        case GL_UNSIGNED_INT:
            glVertexAttribIPointer(
                m_working_attrib_idx,
                count,
                type,
                m_stride,
                (void*)m_working_offset);
            break;

        case GL_FLOAT:                        [[fallthrough]];
        case GL_FIXED:                        [[fallthrough]];
        case GL_INT_2_10_10_10_REV:           [[fallthrough]];
        case GL_UNSIGNED_INT_2_10_10_10_REV:  [[fallthrough]];
        case GL_UNSIGNED_INT_10F_11F_11F_REV:
            glVertexAttribPointer(
                m_working_attrib_idx,
                count,
                type,
                GL_FALSE,
                m_stride,
                (void*)m_working_offset);
            break;

        case GL_DOUBLE:
            glVertexAttribLPointer(
                m_working_attrib_idx,
                count,
                type,
                m_stride,
                (void*)m_working_offset);
            break;
    }

    // finalize attrib
    glEnableVertexAttribArray(m_working_attrib_idx);
    glVertexAttribDivisor(m_working_attrib_idx, m_divisor);

    // advance
    m_working_attrib_idx++;
    m_working_offset += size;
}

void VertexBuffer::bind_data(void* data, GLsizei count, GLenum usage)
{
    use();
    glBufferData(GL_ARRAY_BUFFER, count*m_stride, data, usage);
}


VertexArray::VertexArray(void) :
    m_vao(generate_vertex_array_id())
{ use(); }

VertexArray::~VertexArray(void)
{
    // vbos should be deleted before vao
    m_buffers.clear();

    glDeleteVertexArrays(1, &m_vao);
}

VertexArray::VertexArray(VertexArray&& rhs) :
    m_vao(std::exchange(rhs.m_vao, 0)),
    m_buffers(std::move(rhs.m_buffers))
{}

VertexArray& VertexArray::operator=(VertexArray&& rhs)
{
    this->~VertexArray();
    m_vao = std::exchange(rhs.m_vao, 0);
    m_buffers = std::move(rhs.m_buffers);
    return *this;
}

std::size_t VertexArray::add_vertex_buffer(GLsizei stride, GLuint divisor)
{
    use();
    VertexBuffer buf(stride, divisor);
    m_buffers.emplace_back(std::move(buf));
    return m_buffers.size();
}

VertexBuffer& VertexArray::get_buffer(std::size_t i)
{ return m_buffers.at(i); }
const VertexBuffer& VertexArray::get_buffer(std::size_t i) const
{ return m_buffers.at(i); }

void VertexArray::use(void) const
{ glBindVertexArray(m_vao); }
