#include <stdio.h>
#include "rprintf.h"
#include "serial.h"

void uart_puts(const char *str) {
    esp_printf(my_putc, "%s", str);
}
