#ifndef VM_FILA_REPOSICAO_H
#define VM_FILA_REPOSICAO_H
#include "config.h"

typedef enum { POLITICA_FIFO, POLITICA_LRU } Politica;

typedef struct {
    Politica politica;
    int capacidade;
    int ocupados;               
    int cabeca;
    int cauda;
    int anterior[MAIOR_FILA];
    int seguinte[MAIOR_FILA];
} FilaReposicao;

void fila_iniciar(FilaReposicao *fila, int capacidade, Politica politica);
int fila_reservar(FilaReposicao *fila);
void fila_registrar_uso(FilaReposicao *fila, int slot);

#endif
