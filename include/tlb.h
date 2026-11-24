#ifndef VM_TLB_H
#define VM_TLB_H
#include "fila_reposicao.h"

void tlb_iniciar(Politica politica);
int tlb_buscar(int pagina, int *quadro);
int tlb_inserir(int pagina, int quadro);
void tlb_invalidar_pagina(int pagina);

#endif
