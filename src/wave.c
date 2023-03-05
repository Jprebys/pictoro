#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "wave.h"
#include "logging.h"


#define N_DIRECTIONS 4   // UP LEFT RIGHT DOWN


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


uint32_t cellgrid_get_cell(const CellGrid *grid, const unsigned int x, const unsigned int y)
{
    if (x < grid->cols && y < grid->rows)
    {
        return grid->cells[y * grid->cols + x];
    }
    else
    {
        logger(ERROR, "Attempting to access invalid grid cell (%d, %d). Exiting...\n", x, y);
        exit(EXIT_FAILURE);
    }
}

void pattern_print(Pattern *pattern, int pattern_size)
{

    printf("\nPrinting pattern (count: %d)\n", pattern->count);
    for (int i = 0; i < pattern_size; ++i)
    {
        for (int j = 0; j < pattern_size; ++j)
        {
            printf(" %u", pattern->values[i * pattern_size + j]);
        }
        putchar('\n');
    }
}


bool pattern_equals(Pattern first, Pattern second, int pattern_size)
{
    for (int i = 0; i < pow(pattern_size, 2); ++i)
        if (first.values[i] != second.values[i])
            return false;
    return true;
}


Pattern *pattern_create(int pattern_size)
{
    Pattern *result = malloc(sizeof(Pattern));
    result->values = calloc(pow(pattern_size, 2), sizeof(int));
    result->count = 1;
    return result;
}


void pattern_copy(Pattern *dest, Pattern *src, int pattern_size)
{
    memcpy(dest->values, src->values, pow(pattern_size, 2) * sizeof(int));
    dest->count = src->count;
}


PatternList *patternlist_create(int capacity)
{
    PatternList *result = malloc(sizeof(PatternList));
    size_t *patterns = calloc(capacity, sizeof(size_t));

    result->count = 0;
    result->capacity = capacity;
    result->patterns = patterns;

    return result;
}

void patternlist_append(PatternList *list, int value)
{
    if (list->count > (list->capacity / 2))
    {
        list->capacity *= 2;
        list->patterns = realloc(list->patterns, list->capacity * sizeof(int));
        
    }
    list->patterns[list->count++] = value;
}


void patternlist_free(PatternList *p)
{
    free(p->patterns);
    free(p);
}


int generate_patterns(const CellGrid *grid, const unsigned int max_patterns, const unsigned int pattern_size, Pattern **results)
{

    Pattern *patterns = malloc(sizeof(Pattern) * max_patterns);

    for (size_t i = 0; i < max_patterns; ++i)
    {
        patterns[i].values = malloc(pow(pattern_size, 2) * sizeof(int));
    }

    int pattern_count = 0;
    for (size_t i = 0; i < grid->rows - (pattern_size - 1); ++i)
    {
        for (size_t j = 0; j < grid->cols - (pattern_size - 1); ++j)
        {
            int idx = 0;
            Pattern *new = pattern_create(pattern_size);


            for (size_t k = 0; k < pattern_size; ++k)
            {
                for (size_t l = 0; l < pattern_size; ++l)
                {
                    new->values[idx++] = cellgrid_get_cell(grid, j + l, i + k);
                }
            }

            
            
            // check if already in list
            bool new_cell = true;
            for (int k = 0; k < pattern_count; ++k)
            {

                if (pattern_equals(*new, patterns[k], pattern_size))
                {
                    patterns[k].count++;
                    new_cell = false;
                    break;
                }
            }
            if (new_cell)
            {
                pattern_copy(&patterns[pattern_count++], new, pattern_size);
            }
            free(new->values);
            free(new);
        }
    }

    *results = patterns;
    return pattern_count;
}


int get_pattern_idx(Pattern *patterns, int n_patterns, uint32_t *values, int n)
{
    for (int i = 0; i < n_patterns; ++i)
    {
        for (int j = 0; j < n; j++)
        {
            if (patterns[i].values[j] != values[j])  continue;
            return i;
        }
    }
    return -1; // unreachable
}


bool check_vertical_match(uint32_t *top, uint32_t *bottom, int pattern_size)
{
    for (int i = 0; i < (pattern_size * (pattern_size-1)); ++i)
    {
        if (bottom[i] != top[i + pattern_size])
            return false;
    }
    return true;
}

bool check_horizontal_match(uint32_t *left, uint32_t *right, int pattern_size)
{
    // left  1 2 4 5 7 8     1 3
    // right 0 1 3 4 6 7     0 2

    for (int i = 0; i < pow(pattern_size, 2); ++i)
    {
        if ((i % pattern_size) == (pattern_size - 1)) continue;
        if (right[i] != left[i + 1]) return false;
    }
    return true;
}

void establish_rules(int n_patterns, Pattern *patterns, int n_rules, PatternList *rules[n_rules], int pattern_size)
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


size_t rand_range(const int min_n, const int max_n)
{
    return (size_t) (rand() % (max_n - min_n + 1) + min_n);
}


void run_wfc_algo(const CellGrid *grid, const unsigned int pattern_size, const unsigned int output_width, const unsigned int output_height)
{

    assert(false == 0 && "Warning: false does not equal 0 on this platform");
    srand(0);

    Pattern *patterns;
    size_t max_patterns = (grid->rows - (pattern_size - 1)) * (grid->cols - (pattern_size - 1));

    size_t n_patterns = generate_patterns(grid, max_patterns, pattern_size, &patterns);
    uint32_t result[output_width][output_height];


    int n_rules = n_patterns * N_DIRECTIONS;
    PatternList *rules[n_rules]; 

    establish_rules(n_patterns, patterns, n_rules, rules, pattern_size);
    

    for (size_t i = 0; i < n_patterns; ++i)
    {
        pattern_print(&patterns[i], pattern_size);
        PatternList current_u_rule = *rules[i * 4 + 0];
        printf("Up rules (count %d): ", current_u_rule.count);
        for (size_t j = 0; j < current_u_rule.count; ++j)
            printf("%ld ", current_u_rule.patterns[j]);
        PatternList current_l_rule = *rules[i * 4 + 1];
        printf("\nLeft rules (count %d): ", current_l_rule.count);
        for (size_t j = 0; j < current_l_rule.count; ++j)
            printf("%ld ", current_l_rule.patterns[j]);
        
        putchar('\n');
    }


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

    size_t collapsed_cells = 1;
    while(collapsed_cells < output_grid_size)
    {
        printf("Running for current idx %lu collapsed = %lu\n", current_idx, collapsed_cells);
        changed[current_idx] = false;
        size_t adj_cells[N_DIRECTIONS];
        for (size_t i = 0; i < N_DIRECTIONS; ++i)
            adj_cells[i] = SIZE_MAX;

        if (current_idx >= output_grid_width)                              adj_cells[0] = current_idx - output_grid_width;
        if (current_idx % output_grid_width)                               adj_cells[1] = current_idx - 1;
        if ((current_idx % output_grid_width) != (output_grid_width - 1))  adj_cells[2] = current_idx + 1;
        if (current_idx < output_grid_size - output_grid_width)            adj_cells[3] = current_idx + output_grid_width;
           
        for (size_t x = 0; x < N_DIRECTIONS; ++x)
        {
            size_t new_count = 0;
            size_t adj_idx = adj_cells[x];
            // printf("\nEvaluating adj idx (%lu, %lu)", adj_idx % output_grid_width, adj_idx / output_grid_width);

            if (adj_idx == SIZE_MAX)
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
                printf("\nGrid pos (%ld, %ld) possibilities\n", current_idx % output_grid_width, current_idx / output_grid_width);
                for (size_t i = 0; i < n_patterns; ++i)
                    printf("Pattern %ld: %s\n", i, output_grid[current_idx][i]? "true" : "false");
                printf("\nAdj pos (%ld, %ld) possibilities\n", adj_idx % output_grid_width, adj_idx / output_grid_width);
                for (size_t i = 0; i < n_patterns; ++i)
                    printf("Pattern %ld: %s\n", i, output_grid[adj_idx][i]? "true" : "false");
            // }
            if (!new_count)
            {
                logger(WARNING, "Contradiction reached. Exiting...");
                goto cleanup;
            }            
            else if (counts[adj_idx] != 1 && new_count == 1)
            {
                collapsed_cells++;
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
            // Pick new starting index
            for (size_t i = 0; i < output_grid_size; ++i)
            {
                if (counts[i] > 1)
                {
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
            
        }
    }
    printf("Finished algorithm, collapsed all %lu/%lu cells...\n", collapsed_cells, output_grid_size);
 
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
                        result[i + x][j + y] = pattern.values[y * pattern_size + x];
                    }
                }
            }
            else if (j == output_grid_height - 1)
            {
                for (size_t y = 0; y < pattern_size; ++y)
                    {
                        result[i][j + y] = pattern.values[y * pattern_size];
                    }
            }
            else if (i == output_grid_width - 1)
            {
                for (size_t x = 0; x < pattern_size; ++x)
                    {
                        result[i + x][j] = pattern.values[x];
                    }
            }
            else
            {
                result[i][j] = pattern.values[0];
            }

        }
    }

    for (size_t i = 0; i < output_width; ++i)
    {
        putchar('\n');
        for (size_t j = 0; j < output_height; ++j)
        {
            printf("%02u ", result[i][j]);

        }
    }
    putchar('\n');

    
cleanup:
    for (size_t i = 0; i < n_patterns * N_DIRECTIONS; ++i) 
        patternlist_free(rules[i]);

    for (size_t i = 0; i < max_patterns; ++i) 
        free(patterns[i].values);

    free(patterns);

}





int main(void)
{

    printf("\n\nRunning wave...\n");
    // uint32_t cells[] = {
    //     1, 0, 2, 0,
    //     0, 1, 0, 2,
    //     3, 0, 1, 2
    // };
    // int rows = 3, cols = 4, p_size = 2;
    // CellGrid cell_grid = { .cells = cells, .rows = rows, .cols = cols, .changed = false };
    // run_wfc_algo(&cell_grid, p_size, 24, 32);


    // return 0;

    uint32_t cells2[] = {
        1, 0, 1, 0, 1,
        0, 1, 0, 1, 1,
        1, 0, 1, 1, 1
    };
    int rows = 3, cols = 5, p_size = 2, width = 24, height = 32;
    CellGrid cell_grid2 = { .cells = cells2, .rows = rows, .cols = cols };
    run_wfc_algo(&cell_grid2, p_size, width, height);



    printf("Finished running wave\n");

    return 0;
}