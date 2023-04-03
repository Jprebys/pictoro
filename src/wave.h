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


uint32_t *run_wfc_algo(const CellGrid *grid, const unsigned int pattern_size, const unsigned int output_width, const unsigned int output_height);


#ifdef _UNIT_TEST


typedef struct Pattern
{
    uint32_t *values;
    unsigned int count;
} Pattern;

typedef struct PatternList
{
    size_t *patterns;
    unsigned int count, capacity;
} PatternList;

bool pattern_equals(Pattern first, Pattern second, int pattern_size);
void pattern_copy(Pattern *const dest, const Pattern *const src, const int pattern_size);
bool pattern_in_list(Pattern *const new, Pattern *const patterns, const size_t pattern_count, const size_t pattern_size);
int generate_patterns(const CellGrid *const grid, const unsigned int max_patterns, const unsigned int pattern_size, Pattern **results);
bool check_vertical_match(uint32_t *top, uint32_t *bottom, int pattern_size);
bool check_horizontal_match(uint32_t *left, uint32_t *right, int pattern_size);

#endif  // _UNIT_TEST
#endif  // _H_WAVE