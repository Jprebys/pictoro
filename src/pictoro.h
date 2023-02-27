#ifndef _PICTORO_H
#define _PICTORO_H

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "constants.h"
#include "logging.h"

#define PPM_HEADER_MAXSIZE 256


typedef struct
{
    uint32_t *pixels;
    int width, height;
    bool changed;
} p_frame;


int pictoro_create_frame(p_frame **frame, const int width, const int height)
{
    uint32_t *pixels = malloc(width * height * sizeof(uint32_t));
    p_frame *result = malloc(sizeof(p_frame));

    if (pixels == NULL || result == NULL)
        return 1;

    memset(pixels, 0, width * height * sizeof(uint32_t));

    result->pixels = pixels;
    result->width = width;
    result->height = height;
    result->changed = true;

    *frame = result;
    return 0;
} 


void pictoro_free_frame(p_frame *frame)
{
    free(frame->pixels);
    free(frame);
}


void pictoro_copy_frame(p_frame *dest, p_frame *src)
{
    if (src->width == dest->width && src->height == dest->height)
    {
        memcpy(dest->pixels, src->pixels, src->width * src->height * sizeof(uint32_t));
    }
    else
    {
        logger(ERROR, "Attempting pictoro_copy_frame on differently-sized p_frames. Skipping.");
    }
}


void pictoro_set_pixel(p_frame *frame, const int x, const int y, const uint32_t color)
{
    if (x >= 0 && x < frame->width && y >= 0 && y < frame->height)
    {
        frame->pixels[y * frame->width + x] = color;
        frame->changed = true;
    }
}


uint32_t pictoro_get_pixel(const p_frame *frame, const int x, const int y)
{
    uint32_t color;
    if (x >= 0 && x < frame->width && y >= 0 && y < frame->height)
        color = frame->pixels[y * frame->width + x];
    else
        color = 0;
    return color;
}


void pictoro_fill_frame(p_frame *frame, const uint32_t color)
{
    for (int i = 0; i < frame->width * frame->height; ++i)
    {
        frame->pixels[i] = color;
    }
}


void pictoro_fill_hline(p_frame *frame, const int y, const uint32_t color)
{
    if (y >= 0 && y < frame->height)
    {
        for (int i = 0; i < frame->width; ++i)
        {
            pictoro_set_pixel(frame, i, y, color);
        }
    }
}


void pictoro_fill_vline(p_frame *frame, const int x, const uint32_t color)
{
    if (x >= 0 && x < frame->width)
    {
        for (int i = 0; i < frame->height; ++i)
        {
            pictoro_set_pixel(frame, x, i, color);
        }
    }
}


void pictoro_fill_rect(p_frame *frame, int x, int y, int w, int h, uint32_t color)
{
    if (x < frame->width || y < frame->height)
    {
        for (int i = y; i < y + h; ++i)
        {
            if (i >= frame->height)
                break;
            
            for (int j = x; j < x + w; ++j)
            {
                if (j >= frame->width)
                    break;
                
                pictoro_set_pixel(frame, j, i, color);
            }
        }
    }
}


void pictoro_copy_rect(p_frame *dest, p_frame *src, const int x, const int y, const int h, const int w)
{
    if (x < dest->width || y < dest->height)
    {
        for (int i = y; i < y + h; ++i)
        {
            if (i >= dest->height)
                break;
            
            for (int j = x; j < x + w; ++j)
            {
                if (j >= dest->width)
                    break;
                uint32_t color = pictoro_get_pixel(src, j, i);
                pictoro_set_pixel(dest, j, i, color);
            }
        }
    }
}


void pictoro_fill_circle(p_frame *frame, const int x, const int y, const int radius, const uint32_t color)
{
    for (int i = -radius; i <= radius; ++i)
    {
        for (int j = -radius; j <= radius; ++j)
        {
            if (i * i + j * j <= radius * radius)
                pictoro_set_pixel(frame, x + j, y + i, color);

        }
    }

}


void pictoro_write_str(p_frame *frame, const int x, const int y, 
                          const char *str, const uint32_t color, const uint8_t font_size)
{
    const int column_spacing = font_size * 10;
    const int row_spacing = font_size * 14;

    int cursor_x = x;
    int letter_x_start = x;
    int cursor_y = y;
    int letter_y_start = y;

    for (size_t i = 0; i < strlen(str); ++i)
    {
        char current_char = str[i];

        if (current_char == '\n')
        {
            cursor_x = x;
            letter_x_start = x;
            cursor_y += row_spacing;
            letter_y_start = cursor_y;
            continue;
        }
        else if (current_char < 32)
        {
            printf("[WARN] Unprintable char in pictoro_write_str (value: %d). Skipping.\n", current_char);
            continue;
        }

        int bitmap_idx = current_char - 32;

        for (int j = 12; j >= 0; --j)
        {
            char scanline = bitmap_letters[bitmap_idx][j];
            for (int k = 7; k >= 0; --k)
            {
                if (scanline & (1 << k))
                {
                    pictoro_fill_rect(frame, cursor_x, cursor_y, font_size, font_size, color);
                }
                cursor_x += font_size;
            }
            cursor_x = letter_x_start;
            cursor_y += font_size;
        }
        cursor_y = letter_y_start;
        cursor_x += column_spacing;
        letter_x_start = cursor_x;
    }
}


int pictoro_save_frame(const p_frame *frame, const char *filename)
{
    FILE *f = fopen(filename, "w");
    if (f == NULL)
        return 1;

    char header[PPM_HEADER_MAXSIZE];
    char *ppm_header_fmt = "P6\n%u %u\n255\n";
    snprintf(header, sizeof(header), ppm_header_fmt, frame->width, frame->height);
    header[PPM_HEADER_MAXSIZE - 1] = 0;
    fwrite(header, sizeof(char), strlen(header), f);

    for (int i = 0; i < frame->width * frame->height; ++i)
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