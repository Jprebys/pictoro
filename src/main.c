#include <stdio.h>
#include <stdlib.h>

#include "pictoro.h"


void error_and_die(char *reason)
{
    perror(reason);
    exit(EXIT_FAILURE);
}


int main()
{
    size_t width, height;
    p_frame *frame;

    width = 600;
    height = 400; 

    if (pictoro_create_frame(&frame, width, height))
        error_and_die("create_frame");
    
    // Fill green and add purple rectangle
    pictoro_fill_frame(frame, 0x00FF00FF);
    pictoro_fill_rect(frame, 100, 100, 500, 400, 0xFF00FFFF);

    if (pictoro_save_frame(frame, "frame.ppm"))
        error_and_die("save_frame");

    printf("Successfully created and saved frame\n");

    pictoro_free_frame(frame);
    return 0;
}