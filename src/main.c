#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <SDL2/SDL.h>

#include "logging.h"
#include "pictoro.h"

#define FRAME_RATE    100
#define FRAME_TIME_MS 1000.0f / FRAME_RATE
#define MOUSE_SIZE    50
#define MOUSE_COLOR   0x808080FF


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


void check_time(const uint64_t frame_start)
{
    uint64_t frame_end = SDL_GetPerformanceCounter();
    float elapsed_ms = (frame_end - frame_start) / (float)SDL_GetPerformanceFrequency() * 1000.0f;
    double delay_time = floor(FRAME_TIME_MS - elapsed_ms);
    if (delay_time > 0)
        SDL_Delay(delay_time);
}


void update_texture(p_frame *frame, SDL_Texture *buffer, SDL_Renderer *renderer)
{
    int *tex_pixels;
    int tex_pitch;

    SDL_LockTexture(buffer, NULL, (void **)&tex_pixels, &tex_pitch);
    memcpy(tex_pixels, frame->pixels, frame->width * frame->height * sizeof(uint32_t));
    SDL_UnlockTexture(buffer);
    SDL_RenderCopy(renderer, buffer, NULL, NULL);
    SDL_RenderPresent(renderer);
    frame->changed = false;
}


void draw_mouse(p_frame *frame)
{
    int mouse_x, mouse_y;

    // Check if mouse has moved since last call
    SDL_GetRelativeMouseState(&mouse_x, &mouse_y);
    if (mouse_x || mouse_y)
    {
        SDL_GetMouseState(&mouse_x, &mouse_y);
        pictoro_fill_circle(frame, mouse_x, mouse_y, MOUSE_SIZE, MOUSE_COLOR);
    }
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

    logger(DEBUG, "Creating window");
    SDL_Window *window = SDL_CreateWindow("Drawing Buddy",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          frame->width, frame->height, 0);

    if (window == NULL)
        error_and_die("Create window");    

    logger(DEBUG, "Creating window surface");                                   

    SDL_Surface *window_surface = SDL_GetWindowSurface(window);

    if (window_surface == NULL)
        error_and_die("GetWindowSurface");

    SDL_Renderer *renderer = SDL_CreateRenderer(window,
                                                -1, 
                                                SDL_RENDERER_ACCELERATED 
                                                // | SDL_RENDERER_PRESENTVSYNC
                                                );
    if (renderer == NULL)
        error_and_die("Get renderer");

    SDL_Texture *buffer = SDL_CreateTexture(renderer, 
                                            SDL_PIXELFORMAT_RGBA8888,
                                            SDL_TEXTUREACCESS_STREAMING,
                                            frame->width, frame->height);
    if (buffer == NULL)
        error_and_die("Create Texture");

    bool run = true;
    SDL_Event e;
    while(run)
    {
        uint64_t frame_start = SDL_GetPerformanceCounter();
        draw_mouse(frame);

        if (frame->changed)
        {
            update_texture(frame, buffer, renderer);
        }

        while(SDL_PollEvent(&e) > 0)
        {
            switch(e.type)
            {
                case SDL_QUIT:
                    run = false;
                    break;
            }
            // SDL_UpdateWindowSurface(window);
        }
        check_time(frame_start);
    }

    logger(INFO, "Cleaning up assets");
    SDL_DestroyRenderer(renderer);
    SDL_DestroyTexture(buffer);
    SDL_DestroyWindow(window);
    pictoro_free_frame(frame);
    return 0;
}