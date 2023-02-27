#ifndef _H_WAVE
#define _H_WAVE

#include <stdbool.h>



typedef struct CellGrid
{
    int *cells;
    int rows, cols;
    bool changed;
} CellGrid;


typedef struct SubCell
{
    int *values;
    int count;
} SubCell;


int get_cell_by_pos(CellGrid *grid, int x, int y);






int get_subcells(CellGrid* grid, int max_subcells, int subcell_size, SubCell **results);


void run_wfc_algo(CellGrid *grid, int subcell_size);




#endif