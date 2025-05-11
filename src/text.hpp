#ifndef TEXTH
#define TEXTH

#include <string_view>
#include <filesystem>

#include <GL/glew.h>
#include <GL/gl.h>
#include <SDL_ttf.h>

#include "program.hpp"
#include "texture.hpp"


class Font
{
public:
    Font(std::filesystem::path path, int fontsize);
    ~Font(void);

public:
    Texture render_text_fast_bitmap(std::string_view text, GLuint fmt);

private:
    TTF_Font* mp_font = nullptr;
};

#endif // TEXTH
