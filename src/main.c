#include <stdio.h>
#include <stdlib.h>

#include "pictoro.h"


void error_and_die(char *reason)
{
    perror(reason);
    exit(EXIT_FAILURE);
}
#include <time.h>
#include <stdlib.h>



int main()
{
    size_t width, height;
    p_frame *frame;

    width = 800;
    height = 600; 

    srand(time(NULL)); 

    if (pictoro_create_frame(&frame, width, height))
        error_and_die("create_frame");
    
    for (size_t i = 0; i < width; i += 20)
    {
        for (size_t j = 0; j < height; j += 20)
        {
            int32_t color = 0xFF;
            color |= (rand() % 0x100) << 8; 
            color |= (rand() % 0x100) << 16; 
            color |= (rand() % 0x100) << 24; 
            pictoro_fill_rect(frame, i, j, 20, 20, color - 0x10101000);
        }
    }

    pictoro_write_str(frame, 200, 200, "This\n  is\n your\nSample", 0xFFFFFFFF, 3);

    if (pictoro_save_frame(frame, "frame.ppm"))
        error_and_die("save_frame");

    printf("Successfully created and saved frame\n");

    pictoro_free_frame(frame);
    return 0;
}