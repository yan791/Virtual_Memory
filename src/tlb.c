#include "config.h"
#include "tlb.h"

typedef struct {
    int valida;
    int pagina;
    int quadro;
} EntradaTLB;

static EntradaTLB tlb[ENTRADAS_TLB];
static FilaReposicao fila_tlb;

void tlb_iniciar(Politica politica)
{
    int i;

    for (i = 0; i < ENTRADAS_TLB; i++) {
        tlb[i].valida = 0;
        tlb[i].pagina = 1;
        tlb[i].quadro = -1;
    }
    fila_iniciar(&fila_tlb, ENTRADAS_TLB, politica);
}

int tlb_buscar(int pagina, int *quadro)
{
    int i;

    for (i = 0; i < ENTRADAS_TLB; i++) {
        if (tlb[i].valida && tlb[i].pagina == pagina) {
            *quadro = tlb[i].quadro;
            fila_registrar_uso(&fila_tlb, i);
            return i;
        }
    }
    return -1;
}

int tlb_inserir(int pagina, int quadro)
{
    int entrada = fila_reservar(&fila_tlb);

    tlb[entrada].valida = 1;
    tlb[entrada].pagina = pagina;
    tlb[entrada].quadro = quadro;
    return entrada;
}

void tlb_invalidar_pagina(int pagina)
{
    int i;

    for (i = 0; i < ENTRADAS_TLB; i++)
        if (tlb[i].valida && tlb[i].pagina == pagina)
            tlb[i].valida = 0;
}
