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
    unsigned int width, height;

    width = 600;
    height = 400; 

    p_frame *frame;
    
    if (pictoro_create_frame(&frame, width, height))
        error_and_die("create_frame");
    
    if (pictoro_save_frame(frame, "pic.ppm"))
        error_and_die("save_frame");

    printf("Successfully created and saved frame\n");
    return 0;
}