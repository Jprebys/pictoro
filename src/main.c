#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <SDL2/SDL.h>

#include "pictoro.h"


void error_and_die(char *reason)
{
    perror(reason);
    exit(EXIT_FAILURE);
}


void make_sample_frame(p_frame *frame)
{
    for (size_t i = 0; i < frame->width; i += 20)
    {
        for (size_t j = 0; j < frame->height; j += 20)
        {
            int32_t color = 0xFF;
            color |= (rand() % 0x100) << 8; 
            color |= (rand() % 0x100) << 16; 
            color |= (rand() % 0x100) << 24; 
            pictoro_fill_rect(frame, i, j, 20, 20, color);
        }
    }
    pictoro_write_str(frame, 200, 200, "This\n  is\n your\nSample", 0xFFFFFFFF, 3);
}


int main()
{
    printf("\n\n");

    p_frame *frame;

    srand(time(NULL)); 

    if (pictoro_create_frame(&frame, 800, 600))
        error_and_die("create_frame");


    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        error_and_die("SDL init");

    printf("[INFO] Creating window\n");
    SDL_Window *window = SDL_CreateWindow("SDL2 Window",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          frame->width, frame->height, 0);

    if (window == NULL)
        error_and_die("Create window");                                       

    SDL_Surface *window_surface = SDL_GetWindowSurface(window);

    if (window_surface == NULL)
        error_and_die("GetWindowSurface");

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL)
        error_and_die("Get renderer");

    SDL_Texture *buffer = SDL_CreateTexture(renderer, 
                                            SDL_PIXELFORMAT_RGBA8888,
                                            SDL_TEXTUREACCESS_STREAMING,
                                            frame->width, frame->height);
    if (buffer == NULL)
        error_and_die("Create Texture");

    int *tex_pixels;
    int tex_pitch;

    bool run = true;
    SDL_Event e;
    while(run)
    {
        make_sample_frame(frame);

        SDL_LockTexture(buffer, NULL, (void **)&tex_pixels, &tex_pitch);
        memcpy(tex_pixels, frame->pixels, frame->width * frame->height * sizeof(uint32_t));
        SDL_UnlockTexture(buffer);
        SDL_RenderCopy(renderer, buffer, NULL, NULL);
        SDL_RenderPresent(renderer);

        while(SDL_PollEvent(&e) > 0)
        {
            switch(e.type)
            {
                case SDL_QUIT:
                    run = false;
                    break;
            }
            SDL_UpdateWindowSurface(window);
        }
        SDL_Delay(250);
    }


    SDL_DestroyTexture(buffer);

    return 0;

    
    if (pictoro_save_frame(frame, "frame.ppm"))
        error_and_die("save_frame");

    printf("[INFO] Successfully created and saved frame\n");

    pictoro_free_frame(frame);
    return 0;
}