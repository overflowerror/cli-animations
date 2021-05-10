#include "compat.h"

#include <stdio.h>
#include <wchar.h>
#include <stdarg.h>
#include <stdlib.h>

int compat_mode = 0;

void enable_compat_mode(int mode) {
    compat_mode |= mode;
}

void disable_compat_mode(int mode) {
    compat_mode &= ~(mode);
}

int cfprintf(FILE* file, char* format, ...) {
    va_list(args);
    
    int result;
    
    va_start(args, format);
    if (compat_mode & PRINT_WCHAR) {
        size_t size = mbstowcs(NULL, format, 0) + 1;
        wchar_t* buffer = malloc(size * sizeof(wchar_t));
        if (buffer == NULL) {
            return -1;
        }
        
        mbstowcs(buffer, format, size);
        
        result = vfwprintf(file, buffer, args);
        
        free(buffer);
    } else {
        result = vfprintf(file, format, args);
    }
    va_end(args);
    
    return result;
}
