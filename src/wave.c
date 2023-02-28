#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "wave.h"
#include "logging.h"


int get_cell_by_pos(CellGrid *grid, int x, int y)
{
    if (x >= 0 && x < grid->cols && y >= 0 && y < grid->rows)
    {
        return grid->cells[y * grid->cols + x];
    }
    else
    {
        logger(ERROR, "Attempting to access invalid grid cell (%d, %d). Exiting...\n", x, y);
        exit(EXIT_FAILURE);
    }
}

void print_pattern(Pattern *pattern, int pattern_size)
{

    printf("\nPrinting pattern (count: %d)\n", pattern->count);
    for (int i = 0; i < pattern_size; ++i)
    {
        for (int j = 0; j < pattern_size; ++j)
        {
            printf(" %d", pattern->values[i * pattern_size + j]);
        }
        putchar('\n');
    }
}


bool patterns_equal(Pattern first, Pattern second, int pattern_size)
{
    bool is_equal = true;
    for (int i = 0; i < pow(pattern_size, 2); ++i)
    {
        is_equal &= (first.values[i] == second.values[i]);
    }
    return is_equal;
}


Pattern *make_new_pattern(int pattern_size)
{
    Pattern *result = malloc(sizeof(Pattern));
    result->values = calloc(pow(pattern_size, 2), sizeof(int));
    result->count = 1;
    return result;
}


void copy_pattern(Pattern *dest, Pattern *src, int pattern_size)
{
    memcpy(dest->values, src->values, pow(pattern_size, 2) * sizeof(int));
    dest->count = src->count;
}


PatternList create_patternlist(int capacity)
{
    PatternList *result = malloc(sizeof(PatternList));
    Pattern *patterns = calloc(capacity, sizeof(Pattern));

    result->count = 0;
    result->capacity = capacity;

    return *result;
}


void patternlist_append(PatternList list, Pattern value)
{
    if (list.count > list.capacity / 2)
    {
        list.capacity *= 2;
        list.patterns = realloc(list.patterns, list.capacity * sizeof(Pattern));
    }
    list.patterns[list.count++] = value;
}



int get_patterns(CellGrid *grid, int max_patterns, int pattern_size, Pattern **results)
{

    Pattern *patterns = malloc(sizeof(Pattern) * max_patterns);

    for (int i = 0; i < max_patterns; ++i)
    {
        patterns[i].values = malloc(pow(pattern_size, 2) * sizeof(int));
    }

    int pattern_count = 0;
    for (int i = 0; i < grid->rows - (pattern_size - 1); ++i)
    {
        for (int j = 0; j < grid->cols - (pattern_size - 1); ++j)
        {
            int idx = 0;
            Pattern *new = make_new_pattern(pattern_size);


            for (int k = 0; k < pattern_size; ++k)
            {
                for (int l = 0; l < pattern_size; ++l)
                {
                    new->values[idx++] = get_cell_by_pos(grid, j + l, i + k);
                }
            }

            
            
            // check if already in list
            bool new_cell = true;
            for (int k = 0; k < pattern_count; ++k)
            {

                if (patterns_equal(*new, patterns[k], pattern_size))
                {
                    patterns[k].count++;
                    new_cell = false;
                    break;
                }
            }
            if (new_cell)
            {
                copy_pattern(&patterns[pattern_count++], new, pattern_size);
            }
            free(new->values);
            free(new);
        }
    }

    *results = patterns;
    return pattern_count;
}


int get_pattern_idx(Pattern *patterns, int n_patterns, int *values, int n)
{
    for (int i = 0; i < n_patterns; ++i)
    {
        
    }
}


void establish_rules(CellGrid *grid, Pattern *patterns, int n_patterns, int pattern_size, Rule *rules)
{
    for (int i = 0; i < grid->rows - (pattern_size - 1); ++i)
    {
        for (int j = 0; j < grid->cols - (pattern_size - 1); ++j)
        {
            int idx = 0;
            int values[(int)pow(pattern_size, 2)];
        }
    }
}


void run_wfc_algo(CellGrid *grid, int pattern_size)
{

    Pattern *patterns;
    int max_patterns = (grid->rows - (pattern_size - 1)) * (grid->cols - (pattern_size - 1));


    int n_patterns = get_patterns(grid, max_patterns, pattern_size, &patterns);

    Rule *rules = calloc(n_patterns, sizeof(Rule));
    establish_rules(grid, patterns, n_patterns, pattern_size, rules);




    


    for (int i = 0; i < n_patterns; ++i)
    {
        print_pattern(&patterns[i], pattern_size);
    }


    for (int i = 0; i < max_patterns; ++i)
    {
        free(patterns[i].values);;
    }
    free(patterns);

}



int main()
{

    printf("\n\nRunning wave...\n");
    int cells[] = {
        1, 0, 2,
        0, 1, 0,
        3, 0, 1
    };

    CellGrid cell_grid = { cells, 3, 3, false };

    run_wfc_algo(&cell_grid, 2);



    printf("Finished running wave\n");

    return 0;


}