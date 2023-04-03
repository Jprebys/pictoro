#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "wave.h"

#ifndef _UNIT_TEST
#define internal static

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


#else 
#define internal
#endif  // ifndef _UNIT_TEST

#define N_DIRECTIONS 4   // UP LEFT RIGHT DOWN
#define N_SYMMETRIES 4   // 4 Rotations


internal uint32_t 
cellgrid_get_cell(const CellGrid *grid, const unsigned int x, const unsigned int y)
{
    if (x < grid->cols && y < grid->rows) {
        return grid->cells[y * grid->cols + x];
    } else {
        // logger(ERROR, "Attempting to access invalid grid cell (%d, %d). Exiting...\n", x, y);
        exit(EXIT_FAILURE);
    }
}

internal void 
pattern_print(Pattern *pattern, int pattern_size)
{
    printf("\nPrinting pattern (count: %d)\n", pattern->count);
    for (int i = 0; i < pattern_size; ++i) {
        for (int j = 0; j < pattern_size; ++j) {
            printf(" %u", pattern->values[i * pattern_size + j]);
        }
        putchar('\n');
    }
}


internal bool
pattern_equals(Pattern first, Pattern second, int pattern_size)
{
    for (int i = 0; i < pattern_size * pattern_size; ++i)
        if (first.values[i] != second.values[i])
            return false;
    return true;
}


// internal Pattern *
// pattern_create(int pattern_size)
// {
//     Pattern *result = malloc(sizeof(Pattern));
//     result->values = calloc(pattern_size * pattern_size, sizeof(uint32_t));
//     result->count = 1;
//     return result;
// }


internal void 
pattern_copy(Pattern *const dest, const Pattern *const src, const int pattern_size)
{
    memcpy(dest->values, src->values, pattern_size * pattern_size * sizeof(uint32_t));
    dest->count = src->count;
}


internal PatternList *
patternlist_create(const int capacity)
{
    PatternList *result = malloc(sizeof(PatternList));
    size_t *patterns = calloc(capacity, sizeof(size_t));

    result->count = 0;
    result->capacity = capacity;
    result->patterns = patterns;

    return result;
}

internal void 
patternlist_append(PatternList *const list, const int value)
{
    if (list->count > (list->capacity / 2)) {
        list->capacity *= 2;
        list->patterns = realloc(list->patterns, list->capacity * sizeof(int));
        list->patterns = realloc(list->patterns, list->capacity * sizeof(int));
        
        list->patterns = realloc(list->patterns, list->capacity * sizeof(int));  
        
    }
    list->patterns[list->count++] = value;
}


internal void 
patternlist_free(PatternList *p)
{
    free(p->patterns);
    free(p);
}


internal bool 
pattern_in_list(Pattern *const new, Pattern *const patterns,
                const size_t pattern_count, const size_t pattern_size)
{
    bool existing_cell = false;

    for (size_t k = 0; k < pattern_count; ++k)
    {

        if (pattern_equals(*new, patterns[k], pattern_size))
        {
            patterns[k].count++;
            existing_cell = true;
            break;
        }
    }
    return existing_cell;
}


internal int 
generate_patterns(const CellGrid *const grid, const unsigned int max_patterns, 
                  const unsigned int pattern_size, Pattern **results)
{

    Pattern *patterns = malloc(sizeof(Pattern) * max_patterns);

    for (size_t i = 0; i < max_patterns; ++i) {
        patterns[i].values = malloc(pattern_size * pattern_size * sizeof(int));
    }

    int pattern_count = 0;
    for (size_t i = 0; i < grid->rows - (pattern_size - 1); ++i) {
        for (size_t j = 0; j < grid->cols - (pattern_size - 1); ++j) {
            int idx = 0;
            Pattern new = {.count = 1};
            uint32_t new_values[pattern_size * pattern_size];
            new.values = new_values;


            for (size_t k = 0; k < pattern_size; ++k) {
                for (size_t l = 0; l < pattern_size; ++l) {
                    new.values[idx++] = cellgrid_get_cell(grid, j + l, i + k);
                }
            }

            
            
            // Check if already in list
            if (!pattern_in_list(&new, patterns, pattern_count, pattern_size)) {
                pattern_copy(&patterns[pattern_count++], &new, pattern_size);
            } else {
                continue;
            }
            // Add all symmetries to list
            for (size_t k = 0; k < N_SYMMETRIES - 1; ++k) {
                uint32_t rotated_values[pattern_size * pattern_size];
                for (size_t y = 0; y < pattern_size; ++y) {
                    for (size_t x1 = 0, x2 = pattern_size - 1; x1 < pattern_size; ++x1, --x2) {
                        rotated_values[x2 * pattern_size + y] = new.values[y * pattern_size + x1];
                    }
                }
                memcpy(new.values, rotated_values, sizeof(rotated_values));
                if (!pattern_in_list(&new, patterns, pattern_count, pattern_size)) {
                    pattern_copy(&patterns[pattern_count++], &new, pattern_size);
                }
            }
        }
    }

    *results = patterns;
    return pattern_count;
}

internal bool 
check_vertical_match(uint32_t *top, uint32_t *bottom, int pattern_size)
{
    for (int i = 0; i < (pattern_size * (pattern_size-1)); ++i) {
        if (bottom[i] != top[i + pattern_size])
            return false;
    }
    return true;
}

internal bool
check_horizontal_match(uint32_t *left, uint32_t *right, int pattern_size)
{
    for (int i = 0; i < pattern_size * pattern_size; ++i) {
        if ((i % pattern_size) == (pattern_size - 1)){
            continue;
        }
        if (right[i] != left[i + 1]) {
            return false;
        }
    }
    return true;
}

internal void
establish_rules(int n_patterns, Pattern *patterns, int n_rules, PatternList *rules[n_rules], int pattern_size)
{
    for (int i = 0; i < n_rules; ++i)
        rules[i] = patternlist_create(10);


    for (int i = 0; i < n_patterns; ++i)
    {
        Pattern pattern = patterns[i];
        PatternList *current_u_rule = rules[i * N_DIRECTIONS + 0];
        PatternList *current_l_rule = rules[i * N_DIRECTIONS + 1];
        PatternList *current_r_rule = rules[i * N_DIRECTIONS + 2];
        PatternList *current_d_rule = rules[i * N_DIRECTIONS + 3];

        for (int j = 0; j < n_patterns; ++j)
        {
            Pattern check = patterns[j];
            // up
            if (check_vertical_match(check.values, pattern.values, pattern_size))
                patternlist_append(current_u_rule, j);

            // left
            if (check_horizontal_match(check.values, pattern.values, pattern_size))
                patternlist_append(current_l_rule, j);
            
            // right
            if (check_horizontal_match(pattern.values, check.values, pattern_size))
                patternlist_append(current_r_rule, j);

            //down
            if (check_vertical_match(pattern.values, check.values, pattern_size))
                patternlist_append(current_d_rule, j);
        }
    }
}


internal size_t
rand_range(const int min_n, const int max_n)
{
    return (size_t) (rand() % (max_n - min_n + 1) + min_n);
}


uint32_t *run_wfc_algo(const CellGrid *grid, const unsigned int pattern_size, const unsigned int output_width, const unsigned int output_height)
{
    srand(time(NULL));

    Pattern *patterns;
    size_t max_patterns = (grid->rows - (pattern_size - 1)) * 
                          (grid->cols - (pattern_size - 1)) * 
                          N_SYMMETRIES;

    size_t n_patterns = generate_patterns(grid, max_patterns, pattern_size, &patterns);
    uint32_t *result = malloc(output_width * output_height * sizeof result[0]);


    int n_rules = n_patterns * N_DIRECTIONS;
    PatternList *rules[n_rules]; 

    establish_rules(n_patterns, patterns, n_rules, rules, pattern_size);
    
    size_t output_grid_width = output_width - (pattern_size - 1);
    size_t output_grid_height = output_height - (pattern_size - 1);
    size_t output_grid_size =  output_grid_width * output_grid_height;
    bool output_grid[output_grid_size][n_patterns];
    size_t counts[output_grid_size];
    bool changed[output_grid_size];
    size_t pattern_nos[output_grid_size];
    for (size_t i = 0; i < output_grid_size; ++i)
    {
        for (size_t j = 0; j < n_patterns; ++j)
        {
            output_grid[i][j] = true; 
        }
        counts[i] = n_patterns;
        changed[i] = false;
        pattern_nos[i] = SIZE_MAX;
    }

    // delete from array by swapping with last element and decrementing count
    // pick random starting place
    size_t random_start = rand_range(0, output_grid_size - 1);
    size_t random_pattern = rand_range(0, n_patterns - 1);

    memset(output_grid[random_start], false, n_patterns * sizeof(bool));
    output_grid[random_start][random_pattern] = true;
    pattern_nos[random_start] = random_pattern;
    counts[random_start] = 1;

    //propagate to neighbors
    size_t current_idx = random_start;

    while(1)
    {
        // printf("Running for current idx %lu\n", current_idx);
        changed[current_idx] = false;
        size_t adj_cells[N_DIRECTIONS];
        for(size_t i = 0; i < N_DIRECTIONS; ++i)
            adj_cells[i] = SIZE_MAX;

        if(current_idx >= output_grid_width)                              adj_cells[0] = current_idx - output_grid_width;
        if(current_idx % output_grid_width)                               adj_cells[1] = current_idx - 1;
        if((current_idx % output_grid_width) != (output_grid_width - 1))  adj_cells[2] = current_idx + 1;
        if(current_idx < output_grid_size - output_grid_width)            adj_cells[3] = current_idx + output_grid_width;
           
        for(size_t x = 0; x < N_DIRECTIONS; ++x)
        {
            size_t new_count = 0;
            size_t adj_idx = adj_cells[x];

            if(adj_idx == SIZE_MAX)
                continue;

            assert(adj_idx < output_grid_size && "Bad adj idx");

            // we need to loop over the current index possible patterns
            // and for each one: mark the adj_idx cell true if tha pattern is possible

            bool possible_adj_patterns[n_patterns];
            memset(possible_adj_patterns, 0, sizeof(possible_adj_patterns));

            // We need to OR together each of the rules for the current adj pattern into one array
            for (size_t i = 0; i < n_patterns; ++i)
            {
                if (output_grid[current_idx][i])
                {
                    PatternList *up_rule = rules[i * N_DIRECTIONS + x];
                    for (size_t j = 0; j < up_rule->count; ++j)
                    {
                        possible_adj_patterns[up_rule->patterns[j]] = true;
        
                    }
                }
            }

            // Now AND those possibilities with the existing adj pattern possibilities
            for (size_t i = 0; i < n_patterns; ++i)
            {
                if (output_grid[adj_idx][i])
                {
                    if (possible_adj_patterns[i])
                    {
                        new_count++;
                    }
                    else
                    {
                        output_grid[adj_idx][i] = false;
                        changed[adj_idx] = true;
                    }
                }
            }
            // if (current_idx % output_grid_width == 0 || adj_idx % output_grid_width == 0)
            // {
                // printf("\nGrid pos (%ld, %ld) possibilities\n", current_idx % output_grid_width, current_idx / output_grid_width);
                // for (size_t i = 0; i < n_patterns; ++i)
                //     printf("Pattern %ld: %s\n", i, output_grid[current_idx][i]? "true" : "false");
                // printf("\nAdj pos (%ld, %ld) possibilities\n", adj_idx % output_grid_width, adj_idx / output_grid_width);
                // for (size_t i = 0; i < n_patterns; ++i)
                //     printf("Pattern %ld: %s\n", i, output_grid[adj_idx][i]? "true" : "false");
            // }
            if (!new_count)
            {
                // logger(WARNING, "Contradiction reached. Exiting...");
                printf("Contradiction reached...\n");
                goto cleanup;
            }            
            else if (counts[adj_idx] != 1 && new_count == 1)
            {
                for (size_t z = 0; z < n_patterns; ++z)
                {
                    if (output_grid[adj_idx][z])
                    {
                        pattern_nos[adj_idx] = z;
                        break;
                    }
                }
            }
            counts[adj_idx] = new_count;
        }

        bool done_propagating = true;
        for (size_t i = 0; i < output_grid_size; ++i)
        {
            if (changed[i])
            {
                current_idx = i;
                done_propagating = false;
                break;
            }
        }
        if (done_propagating)
        {
            bool more_left = false;
            // Pick new starting index
            for (size_t i = 0; i < output_grid_size; ++i)
            {
                // printf("counts for %lu: %lu\n", i, counts[i]);
                if (counts[i] > 1)
                {
                    more_left = true;
                    current_idx = i;
                    for (size_t j = 0; j < n_patterns; ++j)
                    {
                        if (output_grid[current_idx][j])
                        {
                            memset(output_grid[current_idx], 0, n_patterns * sizeof(bool));
                            output_grid[current_idx][j] = true;
                            counts[current_idx] = 1;
                            pattern_nos[current_idx] = j;
                        }
                    }
                    break;
                }
            }
            if(!more_left)
            {
                break;
            }            
        }
    }
    for (size_t i = 0; i < output_grid_width; ++i)
    {
        for (size_t j = 0; j < output_grid_height; ++j)
        {
            size_t pattern_no = pattern_nos[j * output_grid_width + i];

            if(pattern_no >= n_patterns)
            {
                printf("bad pattern at pos (%lu, %lu) no: %lu    count: %lu\n", i , j, pattern_no, counts[j * output_grid_width + i]);
                exit(1);
            } 
            Pattern pattern = patterns[pattern_no];
            
            if (j == output_grid_height - 1 && i == output_grid_width - 1)
            {
                for (size_t x = 0; x < pattern_size ;++x)
                {
                    for (size_t y = 0; y < pattern_size; ++y)
                    {
                        result[(i + x) + output_grid_width *(j + y)] = pattern.values[y * pattern_size + x];
                    }
                }
            }
            else if (j == output_grid_height - 1)
            {
                for (size_t y = 0; y < pattern_size; ++y)
                    {
                        result[(i) + output_grid_width *(j + y)] = pattern.values[y * pattern_size];
                    }
            }
            else if (i == output_grid_width - 1)
            {
                for (size_t x = 0; x < pattern_size; ++x)
                    {
                        result[(i + x) + output_grid_width *(j)] = pattern.values[x];
                    }
            }
            else
            {
                result[(i) + output_grid_width *(j)] = pattern.values[0];
            }

        }
    }

    for (size_t i = 0; i < output_width; ++i)
    {
        putchar('\n');
        for (size_t j = 0; j < output_height; ++j)
        {
            printf("%02u ", result[(i) + output_grid_width *(j)]);

        }
    }
    putchar('\n');

    
cleanup:
    for (size_t i = 0; i < n_patterns * N_DIRECTIONS; ++i) 
        patternlist_free(rules[i]);

    for (size_t i = 0; i < max_patterns; ++i) 
        free(patterns[i].values);

    free(patterns);
    return result;
}




