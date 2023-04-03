#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <SDL2/SDL.h>

#include "logging.h"
#include "pictoro.h"
#include "wave.h"

#define FRAME_RATE    30
#define FRAME_TIME_MS 1000.0f / FRAME_RATE
#define N_CELL_ROWS 8
#define N_CELL_COLS 8
#define BLACK 0x000000FF
#define WHITE 0xFFFFFFFF
#define RED   0xFF0000FF
#define GREEN 0x00FF00FF
#define BLUE  0x0000FFFF


typedef enum DrawState
{
    NORMAL,
    DRAWING
} DrawState;


static const uint32_t draw_colors[] = {BLACK, WHITE, RED, GREEN, BLUE};


DrawState draw_state = NORMAL;
static unsigned int draw_colors_idx = 0;
static const size_t n_draw_colors = sizeof(draw_colors) / sizeof(uint32_t);
bool draw_tool_changed = false;
unsigned int draw_size = 5;


void error_and_die(char *reason)
{
    perror(reason);
    exit(EXIT_FAILURE);
}


void make_sample_frame(p_frame *frame)
{
    for (int i = 0; i < frame->width; i += 20) {
        for (int j = 0; j < frame->height; j += 20) {
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
    if (delay_time > 0) {
        SDL_Delay(delay_time);
    }
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


void make_grid(p_frame *frame, const double cell_width, const double cell_height)
{
    for (double i = 0; i < frame->height; i += cell_height) {
        pictoro_fill_hline(frame, i, WHITE);
    }
    for (double j = 0; j < frame->width; j += cell_width) {
        pictoro_fill_vline(frame, j, WHITE);
    }
    pictoro_fill_vline(frame, frame->width - 1, WHITE);
    pictoro_fill_hline(frame, frame->height - 1, WHITE);
}

void draw_grid(p_frame *frame, CellGrid grid, const double cell_width, const double cell_height)
{
    if (grid.changed) {
        pictoro_fill_frame(frame, BLACK);
        for (size_t i = 0; i < grid.rows; ++i) {
            for (size_t j = 0; j < grid.cols; ++j) {
                uint32_t color = grid.cells[i * grid.cols + j];
                pictoro_fill_rect(frame, j * cell_width, i * cell_height, cell_width, cell_height, color);
            }
        }
        make_grid(frame, cell_width, cell_height);
        grid.changed = false;
    }
}


void draw_mouse(p_frame *frame, p_frame *background, int *old_mouse_x, int *old_mouse_y)
{
    int mouse_x, mouse_y, delta_x, delta_y;
    const int mouse_border = 2;
    
    // Check if mouse has moved since last call
    SDL_GetRelativeMouseState(&delta_x, &delta_y);
    if ((delta_x || delta_y) || draw_tool_changed)
    {
        draw_tool_changed = false;
        SDL_GetMouseState(&mouse_x, &mouse_y);
        if (draw_state != DRAWING)
        {
            pictoro_copy_rect(frame, background,
                              *old_mouse_x - draw_size - mouse_border,
                              *old_mouse_y - draw_size - mouse_border,
                              2 * (draw_size + mouse_border),
                              2 * (draw_size + mouse_border));
        }
        *old_mouse_x = mouse_x;
        *old_mouse_y = mouse_y;
        
        pictoro_fill_circle(frame, mouse_x, mouse_y, draw_size, draw_colors[draw_colors_idx]);
    }
}


int main()
{
    int width, height;
    p_frame *frame;

    width = 500;
    height = 500;

    pictoro_create_frame(&frame, width, height);
    pictoro_fill_frame(frame, BLACK);


    const double cell_width = ceil((double)width / N_CELL_COLS);
    const double cell_height = ceil((double)height / N_CELL_ROWS);
    make_grid(frame, cell_width, cell_height);


    uint32_t cells[N_CELL_COLS * N_CELL_ROWS] = {};
    CellGrid cell_grid = {.rows = N_CELL_ROWS, 
                          .cols = N_CELL_COLS, 
                          .cells = cells, 
                          .changed = true};



    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        error_and_die("SDL init");

    logger(DEBUG, "Creating window");
    SDL_Window *window = SDL_CreateWindow("Drawing Buddy",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          frame->width, frame->height, 0);

    if (!window)
        error_and_die("Create window");   

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
        error_and_die("Get renderer");

    SDL_Texture *buffer = SDL_CreateTexture(renderer, 
                                            SDL_PIXELFORMAT_RGBA8888,
                                            SDL_TEXTUREACCESS_STREAMING,
                                            frame->width, frame->height);
    if (!buffer)
        error_and_die("Create Texture");

    int mouse_x, mouse_y;
    int grid_x, grid_y;

    bool run = true;
    SDL_Event e;
    while(run) {
        uint64_t frame_start = SDL_GetPerformanceCounter();

        draw_grid(frame, cell_grid, cell_width, cell_height);

        if (frame->changed)
            update_texture(frame, buffer, renderer);

        while(SDL_PollEvent(&e) > 0) {
            switch(e.type) {
            case SDL_QUIT: {
                run = false;
            }
            break;
            case SDL_MOUSEBUTTONDOWN: {
                SDL_GetMouseState(&mouse_x, &mouse_y);
                grid_x = mouse_x / cell_width;
                grid_y = mouse_y / cell_height;
                if (grid_x >= 0 && grid_x < N_CELL_COLS && grid_y >= 0 && grid_y < N_CELL_ROWS)
                {
                    cell_grid.cells[grid_y * N_CELL_COLS + grid_x] = draw_colors[draw_colors_idx];
                    cell_grid.changed = true;
                } 
            }
            break;
            case SDL_KEYDOWN: {
                if (e.key.keysym.sym == SDLK_c)
                {
                    draw_colors_idx = (draw_colors_idx + 1) % n_draw_colors;
                    printf("Draw color index: %u    draw color: %08X\n", draw_colors_idx, draw_colors[draw_colors_idx]);
                }
                if (e.key.keysym.sym == SDLK_RETURN)
                {
                    run = false;
                }
            }
            break;
            }
        }
        check_time(frame_start);
    }

    size_t output_p_width = 30;
    size_t output_p_height = 20;
    uint32_t *result = run_wfc_algo(&cell_grid, 2, output_p_width, output_p_height);


    size_t output_pixel_size = 50;
    size_t output_win_width = output_p_width * output_pixel_size;
    size_t output_win_height = output_p_height * output_pixel_size;

    p_frame *output_frame;

    pictoro_create_frame(&output_frame, 
                         output_p_width * output_pixel_size, 
                         output_p_height * output_pixel_size);
    pictoro_fill_frame(frame, BLACK);

    SDL_Window *out_window = SDL_CreateWindow("Result", 
                                             SDL_WINDOWPOS_CENTERED, 
                                             SDL_WINDOWPOS_CENTERED,
                                             output_win_width,
                                             output_win_height, 0);

    SDL_Renderer *output_renderer = SDL_CreateRenderer(out_window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture *output_texture = SDL_CreateTexture(output_renderer,
                                                    SDL_PIXELFORMAT_RGBA8888,
                                                    SDL_TEXTUREACCESS_STREAMING,
                                                    output_win_height, output_win_width);

    for (size_t i = 0; i < output_p_width; ++i) {
        for (size_t j = 0; j < output_p_height; ++j) {
            uint32_t color = result[j * output_p_width + i];
            printf("filling position (%zu, %zu) with color %u\n", i, j, color);

            pictoro_fill_rect(output_frame, 
                              i * output_pixel_size, j * output_pixel_size,
                              output_pixel_size, output_pixel_size,
                              color);
        }
    }
    update_texture(output_frame, output_texture, output_renderer);
    SDL_Delay(5000);


    SDL_DestroyRenderer(renderer);
    SDL_DestroyRenderer(output_renderer);
    SDL_DestroyTexture(buffer);
    SDL_DestroyTexture(output_texture);
    SDL_DestroyWindow(window);
    SDL_DestroyWindow(out_window);
    pictoro_free_frame(frame);
    pictoro_free_frame(output_frame);
    free(result);
}


// int main2()
// {
//     printf("\n\n");

//     p_frame *frame, *background;

//     srand(time(NULL)); 

//     if (pictoro_create_frame(&frame, 800, 600))
//         error_and_die("create_frame");

//     pictoro_create_frame(&background, 800, 600);

//     make_sample_frame(frame);
//     pictoro_copy_frame(frame, background);

//     if (SDL_Init(SDL_INIT_VIDEO) < 0)
//         error_and_die("SDL init");

//     logger(DEBUG, "Creating window");
//     SDL_Window *window = SDL_CreateWindow("Drawing Buddy",
//                                           SDL_WINDOWPOS_CENTERED,
//                                           SDL_WINDOWPOS_CENTERED,
//                                           frame->width, frame->height, 0);

//     if (window == NULL)
//         error_and_die("Create window");    

//     logger(DEBUG, "Creating window surface");                                   

//     SDL_Surface *window_surface = SDL_GetWindowSurface(window);

//     if (window_surface == NULL)
//         error_and_die("GetWindowSurface");

//     SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
//     if (renderer == NULL)
//         error_and_die("Get renderer");

//     SDL_Texture *buffer = SDL_CreateTexture(renderer, 
//                                             SDL_PIXELFORMAT_RGBA8888,
//                                             SDL_TEXTUREACCESS_STREAMING,
//                                             frame->width, frame->height);
//     if (buffer == NULL)
//         error_and_die("Create Texture");


//     int old_mouse_x, old_mouse_y;
//     SDL_ShowCursor(SDL_DISABLE);
//     bool run = true;
//     SDL_Event e;
//     while(run)
//     {
//         uint64_t frame_start = SDL_GetPerformanceCounter();
//         draw_mouse(frame, background, &old_mouse_x, &old_mouse_y);

//         if (frame->changed)
//         {
//             update_texture(frame, buffer, renderer);
//         }

//         while(SDL_PollEvent(&e) > 0)
//         {
//             switch(e.type)
//             {
//                 case SDL_QUIT:
//                     run = false;
//                     break;
//                 case SDL_MOUSEBUTTONDOWN:
//                     draw_state = DRAWING;
//                     break;
//                 case SDL_MOUSEBUTTONUP:
//                     draw_state = NORMAL;
//                     pictoro_copy_frame(background, frame);
//                     break;
//                 case SDL_KEYDOWN:
//                     if (e.key.keysym.sym == SDLK_c)
//                     {
//                         draw_colors_idx = (draw_colors_idx + 1) % n_draw_colors;
//                         draw_tool_changed = true;
//                     }
//                     else if (e.key.keysym.sym == SDLK_DOWN && draw_size > 1)
//                     {
//                         draw_size--; 
//                         draw_tool_changed = true;
//                     }
//                     else if (e.key.keysym.sym == SDLK_UP && draw_size < 20)
//                     {
//                         draw_size++;
//                         draw_tool_changed = true;
//                     }
//             }
//         }
//         check_time(frame_start);
//     }

//     logger(INFO, "Cleaning up assets");
//     SDL_DestroyRenderer(renderer);
//     SDL_DestroyTexture(buffer);
//     SDL_DestroyWindow(window);
//     pictoro_free_frame(frame);
//     pictoro_free_frame(background);
//     return 0;
// }
