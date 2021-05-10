#ifndef COMPAT_H
#define COMPAT_H

#include <stdio.h>

#define PRINT_WCHAR (1<<0)

void enable_compat_mode(int);
void disable_compat_mode(int);

int cfprintf(FILE* file, char* format, ...);

#define cprintf(...) cfprintf(stdout, __VA_ARGS__)

#ifdef SUBSTITUDE_PRINTS
    #define printf(...) cprintf(__VA_ARGS__)
    #define fprintf(...) cfprintf(__VA_ARGS__)
#endif
    
#endif
