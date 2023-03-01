#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

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
        PatternList *current_u_rule = rules[i * 4 + 0];
        PatternList *current_l_rule = rules[i * 4 + 1];
        PatternList *current_r_rule = rules[i * 4 + 2];
        PatternList *current_d_rule = rules[i * 4 + 3];

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


void run_wfc_algo(const CellGrid *grid, const unsigned int pattern_size, const unsigned int output_width, const unsigned int output_height)
{

    Pattern *patterns;
    int max_patterns = (grid->rows - (pattern_size - 1)) * (grid->cols - (pattern_size - 1));

    int n_patterns = generate_patterns(grid, max_patterns, pattern_size, &patterns);


    int n_rules = n_patterns * N_DIRECTIONS;
    PatternList *rules[n_rules]; 

    establish_rules(n_patterns, patterns, n_rules, rules, pattern_size);
    

    for (int i = 0; i < n_patterns; ++i)
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


    




    for (int i = 0; i < n_patterns * N_DIRECTIONS; ++i) 
        patternlist_free(rules[i]);

    for (int i = 0; i < max_patterns; ++i) 
        free(patterns[i].values);

    free(patterns);

}



int main(void)
{

    printf("\n\nRunning wave...\n");
    uint32_t cells[] = {
        1, 0, 2, 0,
        0, 1, 0, 2,
        3, 0, 1, 2
    };
    int rows = 3, cols = 4, p_size = 2;
    CellGrid cell_grid = { .cells = cells, .rows = rows, .cols = cols, .changed = false };
    run_wfc_algo(&cell_grid, p_size, 24, 32);




    uint32_t cells2[] = {
        1, 0, 1, 0, 1,
        0, 1, 0, 1, 0,
        1, 0, 1, 0, 1
    };
    rows = 3, cols = 5, p_size = 3;
    CellGrid cell_grid2 = { .cells = cells2, .rows = rows, .cols = cols, .changed = false };
    run_wfc_algo(&cell_grid2, p_size, 24, 32);



    printf("Finished running wave\n");

    return 0;
}