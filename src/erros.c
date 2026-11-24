#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

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

const char *motivo(int codigo)
{
    switch (codigo) {
    case ENOENT:  return "arquivo inexistente";
    case EACCES:  return "permissao negada";
    case EISDIR:  return "e um diretorio";
    case ENOTDIR: return "caminho invalido";
    default:      return strerror(codigo);
    }
}
