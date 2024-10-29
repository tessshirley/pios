#include <stdint.h>

#define SYSTEM_CLOCK_HZ 100000

void wait_msec(uint32_t milliseconds) {
    // calculate the number of iterations needed for the specified milliseconds
    uint32_t iterations = (SYSTEM_CLOCK_HZ / 1000) * milliseconds;
    volatile uint32_t count = 0;
    while(count < iterations) {
        count++;
    }
}

void wait_cycles(uint32_t cycles) {
    while(cycles > 0) {
        __asm__ volatile("nop");
	cycles--;
    }
}
