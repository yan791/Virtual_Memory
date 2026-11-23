#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "config.h"
#include "erros.h"
#include "memoria.h"

static signed char memoria_fisica[NUM_QUADROS * TAM_PAGINA];
static int tabela_paginas[NUM_PAGINAS];     
static int pagina_no_quadro[NUM_QUADROS];   
static FilaReposicao fila_quadros;
static FILE *reserva = NULL;                

static FILE *abrir_backing_store(void)
{
    FILE *arquivo = fopen(ARQUIVO_RESERVA, "rb");

    if (arquivo == NULL) {
        erro("nao foi possivel abrir '%s' (ele deve estar no diretorio "
             "atual): %s", ARQUIVO_RESERVA, strerror(errno));
        return NULL;
    }
    return arquivo;
}

int memoria_iniciar(Politica politica)
{
    int i;

    reserva = abrir_backing_store();
    if (reserva == NULL)
        return 0;

    for (i = 0; i < NUM_PAGINAS; i++)
        tabela_paginas[i] = -1;
    for (i = 0; i < NUM_QUADROS; i++)
        pagina_no_quadro[i] = -1;
    fila_iniciar(&fila_quadros, NUM_QUADROS, politica);
    return 1;
}

void memoria_encerrar(void)
{
    if (reserva != NULL) {
        fclose(reserva);
        reserva = NULL;
    }
}

int memoria_consultar(int pagina)
{
    return tabela_paginas[pagina];
}

int memoria_carregar(int pagina)
{
    int quadro = fila_reservar(&fila_quadros);
    int antiga = pagina_no_quadro[quadro];

    if (antiga >= 0)
        tabela_paginas[antiga] = -1;

    if (fseek(reserva, (long)pagina * TAM_PAGINA, SEEK_SET) != 0
        || fread(&memoria_fisica[quadro * TAM_PAGINA], 1, TAM_PAGINA, reserva)
               != (size_t)TAM_PAGINA) {
        return -1;
    }

    tabela_paginas[pagina] = quadro;
    pagina_no_quadro[quadro] = pagina;
    return quadro;
}

void memoria_registrar_acesso(int quadro)
{
    fila_registrar_uso(&fila_quadros, quadro);
}

int memoria_ler(int endereco_fisico)
{
    return memoria_fisica[endereco_fisico];
}
