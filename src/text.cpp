#include "text.hpp"

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>


// error checking for SDL_ttf calls
#define checkTTFError(val) _checkTTFError( (val), #val, __FILE__, __LINE__ )
void _checkTTFError(
    int const code,
    char const *const func,
    const char *const file,
    int const line)
{
    if (code)
    {
        std::cerr << "\nSDL_ttf Error: " << TTF_GetError()
            << "\n...at " << file << ":" << line << " '" << func << "'"
            << std::endl;
        exit(-1);
    }
}


namespace {
static std::size_t sg_TTF_INIT_COUNT = 0;
}

Font::Font(std::filesystem::path path, int fontsize)
{
    if (sg_TTF_INIT_COUNT++ == 0)
        checkTTFError(TTF_Init() == -1); // -1 on error

    mp_font = TTF_OpenFont(path.c_str(), fontsize);
    checkTTFError(mp_font == NULL);
}

Font::~Font(void)
{
    if (mp_font)
        TTF_CloseFont(mp_font);
    mp_font = nullptr;

    if (--sg_TTF_INIT_COUNT == 0)
        TTF_Quit();
}

Texture Font::render_text_fast_bitmap(std::string_view text, GLuint texture_fmt)
{
    // string_view has no null termination
    std::string text_str(text);

    SDL_Surface* surf = TTF_RenderUTF8_Shaded(
        mp_font,
        text_str.c_str(),
        SDL_Color{255,255,255,255}, SDL_Color{0,0,0,255});
    checkTTFError(surf == NULL);
    SDL_LockSurface(surf);

    int w = surf->w, h = surf->h, surf_pitch = surf->pitch;

    // this step:
    //   - removes pitch, since GL cannot use pitch != width
    //   - pads rows to 4-byte alignment for GL (without glPixelStorei)
    //   - flips image y
    int pitch = (w % 4)? ((w/4 + 1) * 4) : w;
    std::vector<unsigned char> pixels(pitch * h);
    unsigned char* surf_pixels = (unsigned char*)surf->pixels;
    for (int y = 0; y < h; y++)
    {
        int surf_y = h - y - 1;
        for (int x = 0; x < w; x++)
        {
            unsigned long pi = (unsigned long)y * pitch + x;
            unsigned long spi = (unsigned long)surf_y * surf_pitch + x;
            pixels[pi] = surf_pixels[spi];
        }
    }

    SDL_FreeSurface(surf);

    Texture tex(texture_fmt);
    tex.set_pixels(w, h, GL_RED, GL_UNSIGNED_BYTE, &pixels[0]);
    return tex;
}


/*
{
    if ((surf != NULL) && (surf->w > 0) && (surf->h > 0))
    {
        SDL_LockSurface(surf);
        Uint8* const pixels = (Uint8*)surf->pixels;
        // blit each pixel to screen
        for (int32_t dy = 0; dy < surf->h; dy++)
            for (int32_t dx = 0; dx < surf->w; dx++)
                if (pixels[dy * surf->pitch + dx])
                    pixel(x+dx, y+dy, c);
        SDL_UnlockSurface(surf);
    }
    // safe to call SDL_FreeSurface with NULL
    SDL_FreeSurface(surf);
}
*/
