#ifndef _PICTORO_H
#define _PICTORO_H

#include <stdint.h>
#include <string.h>

#define PPM_HEADER_MAXSIZE 256


typedef struct
{
    uint32_t *pixels;
    size_t width, height;
} p_frame;


int pictoro_create_frame(p_frame **frame, const size_t width, const size_t height)
{
    uint32_t *pixels = malloc(width * height * sizeof(uint32_t));
    p_frame *result = malloc(sizeof(p_frame));

    if (pixels == NULL || result == NULL)
        return 1;

    memset(pixels, 0, width * height * sizeof(uint32_t));

    result->pixels = pixels;
    result->width = width;
    result->height = height;

    *frame = result;
    return 0;
} 


void pictoro_free_frame(p_frame *frame)
{
    free(frame->pixels);
    free(frame);
}


void pictoro_fill_frame(p_frame *frame, const uint32_t color)
{
    for (size_t i = 0; i < frame->width * frame->height; ++i)
    {
        frame->pixels[i] = color;
    }
}


void pictoro_fill_rect(p_frame *frame, size_t x, size_t y, size_t w, size_t h, uint32_t color)
{
    if (x < frame->width || y < frame->height)
    {
        for (size_t i = y; i < y + h; ++i)
        {
            if (i >= frame->height)
                break;
            
            for (size_t j = x; j < x + w; ++j)
            {
                if (j >= frame->width)
                    break;
                
                frame->pixels[i * frame->width + j] = color;
            }
        }
    }
}


int pictoro_save_frame(const p_frame *frame, const char *filename)
{
    FILE *f = fopen(filename, "w");
    if (f == NULL)
        return 1;

    char header[PPM_HEADER_MAXSIZE];
    char *pam_fmt = "P6\n%u %u\n255\n";
    snprintf(header, sizeof(header), pam_fmt, frame->width, frame->height);
    header[PPM_HEADER_MAXSIZE - 1] = 0;
    fwrite(header, sizeof(char), strlen(header), f);

    for (size_t i = 0; i < frame->width * frame->height; ++i)
    {
        uint32_t pixel = frame->pixels[i];

        uint8_t value[3];
        value[0] = (pixel >> 24) & 0xFF;
        value[1] = (pixel >> 16) & 0xFF;
        value[2] = (pixel >> 8 ) & 0xFF;

        fwrite(value, sizeof(value), 1, f);
    }
    return 0;
}


#endif