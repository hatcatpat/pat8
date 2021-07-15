#include <math.h>
#include <stdio.h>
#include <SDL2/SDL.h>

#include "pat8.h"

extern struct pat8 p8;

#define screen_width 800
#define screen_height 600

static int clamp(int val, int min, int max)
{
    return (val >= min) ? (val <= max) ? val : max : min;
}

struct pat8emu
{
    struct audio
    {
        SDL_AudioSpec spec;
        SDL_AudioDeviceID id;
    } audio;

    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;

    byte quit;
} p8e;

int p8e_init();
void p8e_quit();
byte p8e_load(char *filename);
void p8e_audio(void *u, Uint8 *stream, int len);
void p8e_parse_input(SDL_KeyCode k, byte v);
void p8e_input();
void p8e_draw();

int main(int argc, char **argv);