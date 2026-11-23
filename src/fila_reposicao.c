#include "fila_reposicao.h"

static void fila_desencaixar(FilaReposicao *fila, int slot)
{
    int ant = fila->anterior[slot];
    int seg = fila->seguinte[slot];

    if (ant >= 0)
        fila->seguinte[ant] = seg;
    else
        fila->cabeca = seg;

    if (seg >= 0)
        fila->anterior[seg] = ant;
    else
        fila->cauda = ant;

    fila->anterior[slot] = -1;
    fila->seguinte[slot] = -1;
}

static void fila_encaixar_no_fim(FilaReposicao *fila, int slot)
{
    fila->anterior[slot] = fila->cauda;
    fila->seguinte[slot] = -1;

    if (fila->cauda >= 0)
        fila->seguinte[fila->cauda] = slot;
    else
        fila->cabeca = slot;

    fila->cauda = slot;
}

void fila_iniciar(FilaReposicao *fila, int capacidade, Politica politica)
{
    int i;

    fila->politica = politica;
    fila->capacidade = capacidade;
    fila->ocupados = 0;
    fila->cabeca = -1;
    fila->cauda = -1;
    for (i = 0; i < MAIOR_FILA; i++) {
        fila->anterior[i] = -1;
        fila->seguinte[i] = -1;
    }
}

int fila_reservar(FilaReposicao *fila)
{
    int slot;

    if (fila->ocupados < fila->capacidade) {
        slot = fila->ocupados++;
    } else {
        slot = fila->cabeca;
        fila_desencaixar(fila, slot);
    }
    fila_encaixar_no_fim(fila, slot);
    return slot;
}
