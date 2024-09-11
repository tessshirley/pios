
char glbl[128];


unsigned long get_timer_count() {
     unsigned long *timer_count_register = 0x3f003004;
     return *timer_count_register;
}

void kernel_main() {
    get_timer_count();
    extern int __bss_start, __bss_end;
    char *bssstart, *bssend;
    // zero out the bss segment
    bssstart = &__bss_start;
    bssend = &__bss_end;

    while(bssstart < bssend) {
	*bssstart++ = 0;
    }

    while(1){
    }

}
