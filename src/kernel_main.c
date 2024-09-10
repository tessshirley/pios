

char glbl[128];

void kernel_main() {
    
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
