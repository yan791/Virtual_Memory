#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "erros.h"

static void mostrar_uso(void){
    fputs("uso: ./vm <arquivo_de_enderecos> <fifo|lru> <fifo|lru>\n"
          "  1o argumento: arquivo com um endereco logico (0 a 65535) por linha\n"
          "  2o argumento: algoritmo de substituicao de pagina (fifo ou lru)\n"
          "  3o argumento: algoritmo de substituicao da TLB    (fifo ou lru)\n"
          "exemplo: ./vm address.txt lru fifo\n",
          stderr);
}

typedef enum { POLITICA_FIFO, POLITICA_LRU } Politica;

static int interpretar_politica(const char *texto, Politica *politica)
{
    if (strcmp(texto, "fifo") == 0) {
        *politica = POLITICA_FIFO;
        return 1;
    }
    if (strcmp(texto, "lru") == 0) {
        *politica = POLITICA_LRU;
        return 1;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    Politica politica_pagina;
    Politica politica_tlb;

    if (argc != 4) {
        erro("numero de argumentos invalido: esperados 3, recebidos %d",
             argc > 0 ? argc - 1 : 0);
        mostrar_uso();
        return EXIT_FAILURE;
    }
    if (!interpretar_politica(argv[2], &politica_pagina)) {
        erro("algoritmo de substituicao de pagina invalido: '%s' "
             "(use fifo ou lru)", argv[2]);
        mostrar_uso();
        return EXIT_FAILURE;
    }
    if (!interpretar_politica(argv[3], &politica_tlb)) {
        erro("algoritmo de substituicao da TLB invalido: '%s' "
             "(use fifo ou lru)", argv[3]);
        mostrar_uso();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
