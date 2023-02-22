#ifndef _PICTORO_H
#define _PICTORO_H

#include <stdint.h>
#include <string.h>


typedef struct
{
    uint32_t *pixels;
    unsigned int width, height;
} p_frame;

void pictoro_fill_frame(p_frame *frame, const uint32_t color)
{
    for (size_t i = 0; i < frame->width * frame->height; ++i)
    {
        frame->pixels[i] = color;
    }
}

int pictoro_create_frame(p_frame **frame, const unsigned int width, const unsigned int height)
{
    uint32_t *pixels = malloc(width * height * sizeof(uint32_t));
    p_frame *result = malloc(sizeof(p_frame));

    if (pixels == NULL || result == NULL)
    {
        return 1;
    }

    memset(pixels, 0, width * height * sizeof(uint32_t));

    result->pixels = pixels;
    result->width = width;
    result->height = height;

    *frame = result;
    return 0;
} 

int pictoro_save_frame(p_frame *frame, char *filename)
{
    FILE *f = fopen(filename, "w");
    if (f == NULL)
        return 1;

    

    return 0;
}


#endif