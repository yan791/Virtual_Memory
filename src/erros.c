#define _POSIX_C_SOURCE 200809L

#include <stdarg.h>
#include <stdio.h>

#include "erros.h"

void erro(const char *formato, ...)
{
    va_list args;

    fputs("vm: erro: ", stderr);
    va_start(args, formato);
    vfprintf(stderr, formato, args);
    va_end(args);
    fputc('\n', stderr);
}
