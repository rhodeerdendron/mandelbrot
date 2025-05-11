#include "program.hpp"

#include <iostream>
#include <iomanip>
#include <string>
#include <utility>


static inline void print_shader_log(GLuint id)
{
    // get log length
    int length = 0, maxLength = 0;
    glGetShaderiv(id, GL_INFO_LOG_LENGTH, &maxLength);

    // fetch and print log
    char* log = new char[maxLength];
    glGetShaderInfoLog(id, maxLength, &length, log);
    if (length > 0)
        std::cerr << log << std::endl;
    delete[] log;
}

static inline void print_program_log(GLuint id)
{
    // get log length
    int length = 0, maxLength = 0;
    glGetProgramiv(id, GL_INFO_LOG_LENGTH, &maxLength);

    // fetch and print log
    char* log = new char[maxLength];
    glGetProgramInfoLog(id, maxLength, &length, log);
    if (length > 0)
        std::cerr << log << std::endl;
    delete[] log;
}


// helper methods for reading files

#include <fstream>

struct file_data
{
    file_data(void) : file_data(0) {}
    file_data(int size) : size(size)
    {
        if (size <= 0) return;
        data = new char[size+1];
        data[size] = '\0';
    }
    ~file_data(void) { if (data) delete[] data; }

    file_data(const file_data&) = delete;
    file_data& operator=(const file_data&) = delete;

    file_data(file_data&& rhs) :
        size(std::exchange(rhs.size, 0)),
        data(std::exchange(rhs.data, nullptr))
    {}
    file_data& operator=(file_data&& rhs)
    {
        this->~file_data();
        size = std::exchange(rhs.size, 0);
        data = std::exchange(rhs.data, nullptr);
        return *this;
    }

    int size = 0;
    char* data = nullptr;
};

static inline file_data read_file(std::filesystem::path path)
{
    // load the file
    std::ifstream file(path, std::ios::binary);
    if (!file)
    {
        std::cerr << "failed to open file " << std::quoted(path.c_str()) << std::endl;
        return file_data{};
    }

    // get file size
    file.seekg(0, std::ios::end);
    int length = file.tellg();
    file.seekg(0, std::ios::beg);

    // read data
    file_data data(length);
    file.read(data.data, data.size);
    file.close();
    return data;
}


Program::Program(std::filesystem::path vsrc, std::filesystem::path fsrc)
{
    // read files
    file_data vertfile = read_file(vsrc);
    file_data fragfile = read_file(fsrc);

    if (vertfile.size == 0 || fragfile.size == 0)
    {
        std::cerr << "failed to read shaders for program creation" << std::endl;
        exit(2);
    }

    // compile shaders
    GLuint vertid = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragid = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(vertid, 1, &vertfile.data, &vertfile.size);
    glShaderSource(fragid, 1, &fragfile.data, &fragfile.size);
    glCompileShader(vertid);
    glCompileShader(fragid);

    // check compilation status
    GLint vertsuccess = GL_FALSE;
    GLint fragsuccess = GL_FALSE;
    glGetShaderiv(vertid, GL_COMPILE_STATUS, &vertsuccess);
    glGetShaderiv(fragid, GL_COMPILE_STATUS, &fragsuccess);
    if (vertsuccess != GL_TRUE || fragsuccess != GL_TRUE)
    {
        std::cerr << "failed to compile shaders for program creation";
        std::cerr << "\nvertex shader log: \n";
        print_shader_log(vertid);
        std::cerr << "\nfragment shader log: \n";
        print_shader_log(fragid);

        glDeleteShader(vertid);
        glDeleteShader(fragid);
        exit(3);
    }

    // link program
    GLuint progid = glCreateProgram();
    glAttachShader(progid, vertid);
    glAttachShader(progid, fragid);
    glLinkProgram(progid);

    // check linkage status
    GLint progsuccess = GL_FALSE;
    glGetProgramiv(progid, GL_LINK_STATUS, &progsuccess);
    if (progsuccess != GL_TRUE)
    {
        std::cerr << "failed to link program";
        std::cerr << "\nprogram log:\n";
        print_program_log(progid);

        glDeleteShader(vertid);
        glDeleteShader(fragid);
        glDeleteProgram(progid);
        exit(4);
    }

    // all good!
    m_id = progid;
}

Program::~Program(void)
{
    if (m_id)
        glDeleteProgram(m_id);
}

void Program::use(void) const
{
    glUseProgram(m_id);
}

GLint Program::get_uniform(std::string_view name) const
{
    // std::string_view does not have a null termination
    std::string name_str{ name };

    GLint attr = glGetUniformLocation(m_id, name_str.c_str());
    if (attr == -1)
    {
        std::cerr
            << "could not find uniform " << std::quoted(name)
            << " on program " << m_id
            << std::endl;
    }
    return attr;
}
