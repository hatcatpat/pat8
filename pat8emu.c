#include "pat8emu.h"

int p8e_init()
{
    p8e.window = NULL;
    p8e.renderer = NULL;
    p8e.texture = NULL;
    p8e.quit = 0;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
    {
        printf("[error] sdl failed to init: %s\n", SDL_GetError());
        return 1;
    }
    else
    {
        p8e.window = SDL_CreateWindow("pat8", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen_width, screen_height, SDL_WINDOW_SHOWN);

        if (p8e.window == NULL)
        {
            printf("[error] window failed to create: %s\n", SDL_GetError());
            return 1;
        }

        p8e.renderer = SDL_CreateRenderer(p8e.window, -1, 0);
        if (p8e.renderer == NULL)
        {
            printf("[error] window failed to create: %s\n", SDL_GetError());
            return 1;
        }

        p8e.texture = SDL_CreateTexture(p8e.renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, width, height);
        if (p8e.texture == NULL)
        {
            printf("[error] texture failed to create: %s\n", SDL_GetError());
            return 1;
        }

        p8e.audio.spec.freq = 256;
        p8e.audio.spec.format = AUDIO_U8;
        p8e.audio.spec.channels = 1;
        p8e.audio.spec.callback = p8e_audio;
        p8e.audio.spec.samples = 512;
        p8e.audio.spec.userdata = NULL;
        p8e.audio.id = SDL_OpenAudioDevice(NULL, 0, &p8e.audio.spec, NULL, 0);
        if (!p8e.audio.id)
            SDL_Log("[error] audio failed to open: %s\n", SDL_GetError());
        SDL_PauseAudioDevice(p8e.audio.id, 0);

        while (!p8e.quit)
        {
            double start_time = SDL_GetPerformanceCounter();

            p8e_input();
            p8_eval();
            if (p8.error)
            {
                p8e.quit = 1;
                break;
            }
            p8e_draw();

            double elapsed_time = (SDL_GetPerformanceCounter() - start_time) / (double)SDL_GetPerformanceFrequency() * 1000.0f;
            SDL_Delay(clamp(16.666f - elapsed_time, 0, 1000));
        }
    }

    return 0;
}

void p8e_quit()
{
    SDL_DestroyTexture(p8e.texture);
    SDL_DestroyRenderer(p8e.renderer);
    SDL_DestroyWindow(p8e.window);
    SDL_Quit();
}

byte p8e_load(char *filename)
{
    FILE *file = fopen(filename, "rb");
    if (file == NULL)
    {
        printf("[p8e error] unable to open filename %s\n", filename);
        return 1;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    char *buffer = (char *)malloc(file_size * sizeof(char));
    size_t result = fread(buffer, 1, file_size, file);

    if (file_size > mem_size)
    {
        printf("[p8e error] unable to open filename %s, program too large %li\n", filename, file_size);

        fclose(file);
        free(buffer);
        return 1;
    }

    for (int i = 0; i < file_size; i++)
    {
        printf("%i %x\n", i, (byte)buffer[i]);
        p8.memory[i] = (byte)buffer[i];
    }

    fclose(file);
    free(buffer);

    return 0;
}

void p8e_audio(void *u, Uint8 *stream, int len)
{
    Uint8 *samples = (Uint8 *)stream;
    SDL_memset(stream, 0, len);

    for (int i = 0; i < len; i++)
    {
        // p8_audio();
        // for (byte c = 0; c < 2; c++)
        // *samples++ = p8.audio[c];
    }

    (void)u;
}

void p8e_parse_input(SDL_KeyCode k, byte v)
{
    if (k == '1')
        p8.key[0x0] = v;
    else if (k == '2')
        p8.key[0x1] = v;
    else if (k == '3')
        p8.key[0x2] = v;
    else if (k == '4')
        p8.key[0x3] = v;

    else if (k == 'q')
        p8.key[0x4] = v;
    else if (k == 'w')
        p8.key[0x5] = v;
    else if (k == 'e')
        p8.key[0x6] = v;
    else if (k == 'r')
        p8.key[0x7] = v;

    else if (k == 'a')
        p8.key[0x8] = v;
    else if (k == 's')
        p8.key[0x9] = v;
    else if (k == 'd')
        p8.key[0xa] = v;
    else if (k == 'f')
        p8.key[0xb] = v;

    else if (k == 'z')
        p8.key[0xc] = v;
    else if (k == 'x')
        p8.key[0xd] = v;
    else if (k == 'c')
        p8.key[0xe] = v;
    else if (k == 'v')
        p8.key[0xf] = v;
}

void p8e_input()
{
    SDL_Event evt;
    while (SDL_PollEvent(&evt) != 0)
    {
        if (evt.type == SDL_QUIT)
        {
            p8e.quit = 1;
            break;
        }
        else if (evt.type == SDL_KEYDOWN)
        {
            SDL_KeyCode k = evt.key.keysym.sym;

            if (k == SDLK_ESCAPE)
            {
                SDL_Event q;
                q.type = SDL_QUIT;
                SDL_PushEvent(&q);
            }
            else
            {
                p8e_parse_input(k, 1);
            }
        }
        else if (evt.type == SDL_KEYUP)
        {
            p8e_parse_input(evt.key.keysym.sym, 0);
        }
    }
}

void p8e_draw()
{
    if (p8.draw)
    {
        Uint32 *pixels;
        int pitch = 0;
        SDL_LockTexture(p8e.texture, NULL, (void **)&pixels, &pitch);
        for (int x = 0; x < width; x++)
        {
            for (int y = 0; y < height; y++)
            {
                Uint32 pos = y * (pitch / sizeof(unsigned int)) + x;
                byte pix = p8.video[y * width + x];
                pixels[pos] = (pix << 16) | (pix << 8) | pix;
            }
        }
        SDL_UnlockTexture(p8e.texture);

        SDL_RenderClear(p8e.renderer);
        SDL_RenderCopy(p8e.renderer, p8e.texture, NULL, NULL);
        SDL_RenderPresent(p8e.renderer);

        p8.draw = 0;
    }
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("usage: pat8emu ROM\n");
        return 1;
    }

    if (p8_init() | p8e_load(argv[1]))
        return 1;

    p8e_init();
    p8e_quit();

    return 0;
}