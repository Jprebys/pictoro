#ifndef _H_WAVE
#define _H_WAVE

#include <stdbool.h>



typedef struct CellGrid
{
    int *cells;
    int rows, cols;
    bool changed;
} CellGrid;


typedef struct Pattern
{
    int *values;
    int count;
} Pattern;


// directions: upleft, up, upright, left, right, downleft, down, downright


// For each pattern in our grid we need to have
// a list of every potential pattern for each cardinal direction

typedef struct PatternList
{
    Pattern *patterns;
    int count, capacity;
} PatternList;


typedef struct Rule
{
    Pattern pattern;
    PatternList adjacencies[8];
} Rule;


int get_cell_by_pos(CellGrid *grid, int x, int y);

int get_patterns(CellGrid* grid, int max_patterns, int pattern_size, Pattern **results);
void run_wfc_algo(CellGrid *grid, int pattern_size);




#endif