#include <stdio.h>
#include "rprintf.h"

#define MU_IO (volatile unsigned int *) (0x3F215040)

void my_putc(int data) {
    // print a single character to the serial port
    *MU_IO = data;
}

