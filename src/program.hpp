#ifndef PROGRAMH
#define PROGRAMH

#include <filesystem>
#include <string_view>

#include <GL/glew.h>
#include <GL/gl.h>


class Program
{
public:
    Program(std::filesystem::path vert, std::filesystem::path frag);
    ~Program(void);

public:
    GLuint get_id(void) const { return m_id; }

    GLint get_uniform(std::string_view name) const;

    void use(void) const;

private:
    GLuint m_id = 0;
};

#endif // PROGRAMH
