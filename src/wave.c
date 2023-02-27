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

void print_subcell(SubCell *subcell, int subcell_size)
{

    printf("\nPrinting subcell (count: %d)\n", subcell->count);
    for (int i = 0; i < subcell_size; ++i)
    {
        for (int j = 0; j < subcell_size; ++j)
        {
            printf(" %d", subcell->values[i * subcell_size + j]);
        }
        putchar('\n');
    }
}

bool subcells_equal(SubCell first, SubCell second, int subcell_size)
{
    bool is_equal = true;
    for (int i = 0; i < pow(subcell_size, 2); ++i)
    {
        is_equal &= (first.values[i] == second.values[i]);
    }
    return is_equal;
}

SubCell *make_new_subcell(int subcell_size)
{
    SubCell *result = malloc(sizeof(SubCell));
    result->values = malloc(pow(subcell_size, 2) * sizeof(int));
    result->count = 1;
    return result;
}


void copy_subcell(SubCell *dest, SubCell *src, int subcell_size)
{
    memcpy(dest->values, src->values, pow(subcell_size, 2) * sizeof(int));
    dest->count = src->count;
}


int get_subcells(CellGrid *grid, int max_subcells, int subcell_size, SubCell **results)
{

    SubCell *subcells = malloc(sizeof(SubCell) * max_subcells);

    for (int i = 0; i < max_subcells; ++i)
    {
        subcells[i].values = malloc(pow(subcell_size, 2) * sizeof(int));
    }

    int subcell_count = 0;
    for (int i = 0; i < grid->rows - (subcell_size - 1); ++i)
    {
        for (int j = 0; j < grid->cols - (subcell_size - 1); ++j)
        {
            int idx = 0;
            SubCell *new = make_new_subcell(subcell_size);


            for (int k = 0; k < subcell_size; ++k)
            {
                for (int l = 0; l < subcell_size; ++l)
                {
                    new->values[idx++] = get_cell_by_pos(grid, j + l, i + k);
                }
            }
            
            // check if already in list
            bool new_cell = true;
            for (int k = 0; k < subcell_count; ++k)
            {

                if (subcells_equal(*new, subcells[k], subcell_size))
                {
                    subcells[k].count++;
                    new_cell = false;
                    break;
                }
            }
            if (new_cell)
            {
                copy_subcell(&subcells[subcell_count++], new, subcell_size);
            }
            free(new->values);
            free(new);
        }
    }

    *results = subcells;
    return subcell_count;
}


void run_wfc_algo(CellGrid *grid, int subcell_size)
{

    SubCell *subcells;
    int max_subcells = (grid->rows - (subcell_size - 1)) * (grid->cols - (subcell_size - 1));


    int n_subcells = get_subcells(grid, max_subcells, subcell_size, &subcells);






    


    for (int i = 0; i < n_subcells; ++i)
    {
        print_subcell(&subcells[i], subcell_size);
    }


    for (int i = 0; i < max_subcells; ++i)
    {
        free(subcells[i].values);;
    }
    free(subcells);

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