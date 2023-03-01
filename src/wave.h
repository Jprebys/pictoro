#ifndef _H_WAVE
#define _H_WAVE

#include <stdbool.h>
#include <stdint.h>


typedef struct CellGrid
{
    uint32_t *cells;
    unsigned int rows, cols;
    bool changed;
} CellGrid;


void run_wfc_algo(const CellGrid *grid, const unsigned int pattern_size, const unsigned int output_width, const unsigned int output_height);


#endif