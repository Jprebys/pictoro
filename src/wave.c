#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "wave.h"
#include "logging.h"


#define N_DIRECTIONS 4   // UP LEFT RIGHT DOWN


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
    for (int i = 0; i < pow(pattern_size, 2); ++i)
        if (first.values[i] != second.values[i])
            return false;
    return true;
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


PatternList *create_patternlist(int capacity)
{
    PatternList *result = malloc(sizeof(PatternList));
    int *patterns = calloc(capacity, sizeof(int));

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
        for (int j = 0; j < n; j++)
        {
            if (patterns[i].values[j] != values[j])  continue;
            return i;
        }
    }

    return -1; // unreachable
}


bool check_vertical_match(int *top, int *bottom, int pattern_size)
{
    for (int i = 0; i < (pattern_size * (pattern_size-1)); ++i)
    {
        if (bottom[i] != top[i + pattern_size])
            return false;
    }
    return true;
}

bool check_horizontal_match(int *left, int *right, int pattern_size)
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
        rules[i] = create_patternlist(10);


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

void run_wfc_algo(CellGrid *grid, int pattern_size)
{

    Pattern *patterns;
    int max_patterns = (grid->rows - (pattern_size - 1)) * (grid->cols - (pattern_size - 1));

    int n_patterns = get_patterns(grid, max_patterns, pattern_size, &patterns);


    int n_rules = n_patterns * N_DIRECTIONS;
    PatternList *rules[n_rules]; 

    establish_rules(n_patterns, patterns, n_rules, rules, pattern_size);
    

    for (int i = 0; i < n_patterns; ++i)
    {
        print_pattern(&patterns[i], pattern_size);
        PatternList current_u_rule = *rules[i * 4 + 0];
        printf("Up rules (count %d): ", current_u_rule.count);
        for (int j = 0; j < current_u_rule.count; ++j)
            printf("%d ", current_u_rule.patterns[j]);
        PatternList current_l_rule = *rules[i * 4 + 1];
        printf("\nLeft rules (count %d): ", current_l_rule.count);
        for (int j = 0; j < current_l_rule.count; ++j)
            printf("%d ", current_l_rule.patterns[j]);
        
        putchar('\n');
    }


    for (int i = 0; i < n_patterns * N_DIRECTIONS; ++i) 
        patternlist_free(rules[i]);

    for (int i = 0; i < max_patterns; ++i) free(patterns[i].values);
    free(patterns);

}



int main(void)
{

    printf("\n\nRunning wave...\n");
    int cells[] = {
        1, 0, 2, 0,
        0, 1, 0, 2,
        3, 0, 1, 2
    };
    int rows = 3, cols = 4, p_size = 2;
    CellGrid cell_grid = { .cells = cells, .rows = rows, .cols = cols, .changed = false };
    run_wfc_algo(&cell_grid, p_size);




    int cells2[] = {
        1, 0, 1, 0, 1,
        0, 1, 0, 1, 0,
        1, 0, 1, 0, 1
    };
    rows = 3, cols = 5, p_size = 3;
    CellGrid cell_grid2 = { .cells = cells2, .rows = rows, .cols = cols, .changed = false };
    run_wfc_algo(&cell_grid2, p_size);



    printf("Finished running wave\n");

    return 0;


}