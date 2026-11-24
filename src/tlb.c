#define _POSIX_C_SOURCE 200809L
#include <pthread.h>
#include <string.h>
#include "config.h"
#include "erros.h"
#include "tlb.h"

typedef struct {
    int valida;
    int pagina;
    int quadro;
} EntradaTLB;

static EntradaTLB tlb[ENTRADAS_TLB];
static FilaReposicao fila_tlb;

static struct {
    pthread_mutex_t trava;
    pthread_cond_t  largada;   
    pthread_cond_t  chegada;   
    unsigned long   rodada;
    int             alvo;       
    int             faltam;     
    int             achou;      
    int             encerrar;
} painel;

static pthread_t vigias[ENTRADAS_TLB];
static int vigias_criadas = 0;
static int painel_pronto = 0;

static void *vigia(void *argumento)
{
    const EntradaTLB *minha = argumento;
    const int indice = (int)(minha - tlb);
    unsigned long ultima_rodada = 0;
    int alvo;
    int casou;

    pthread_mutex_lock(&painel.trava);
    for (;;) {
        while (painel.rodada == ultima_rodada && !painel.encerrar)
            pthread_cond_wait(&painel.largada, &painel.trava);
        if (painel.encerrar)
            break;

        ultima_rodada = painel.rodada;
        alvo = painel.alvo;
        pthread_mutex_unlock(&painel.trava);

        casou = minha->valida && minha->pagina == alvo;

        pthread_mutex_lock(&painel.trava);
        if (casou)
            painel.achou = indice;
        if (--painel.faltam == 0)
            pthread_cond_signal(&painel.chegada);
    }
    pthread_mutex_unlock(&painel.trava);
    return NULL;
}

void tlb_encerrar(void)
{
    int i;

    if (!painel_pronto)
        return;

    pthread_mutex_lock(&painel.trava);
    painel.encerrar = 1;
    pthread_cond_broadcast(&painel.largada);
    pthread_mutex_unlock(&painel.trava);

    for (i = 0; i < vigias_criadas; i++)
        pthread_join(vigias[i], NULL);
    vigias_criadas = 0;

    pthread_cond_destroy(&painel.chegada);
    pthread_cond_destroy(&painel.largada);
    pthread_mutex_destroy(&painel.trava);
    painel_pronto = 0;
}

static int iniciar_vigias(void)
{
    int i;
    int codigo;

    painel.rodada = 0;
    painel.alvo = -1;
    painel.faltam = 0;
    painel.achou = -1;
    painel.encerrar = 0;

    if (pthread_mutex_init(&painel.trava, NULL) != 0) {
        erro("nao foi possivel criar o mutex da TLB");
        return 0;
    }
    if (pthread_cond_init(&painel.largada, NULL) != 0) {
        pthread_mutex_destroy(&painel.trava);
        erro("nao foi possivel criar a variavel de condicao da TLB");
        return 0;
    }
    if (pthread_cond_init(&painel.chegada, NULL) != 0) {
        pthread_cond_destroy(&painel.largada);
        pthread_mutex_destroy(&painel.trava);
        erro("nao foi possivel criar a variavel de condicao da TLB");
        return 0;
    }
    painel_pronto = 1;

    for (i = 0; i < ENTRADAS_TLB; i++) {
        codigo = pthread_create(&vigias[i], NULL, vigia, &tlb[i]);
        if (codigo != 0) {
            erro("nao foi possivel criar a thread da entrada %d da TLB: %s",
                 i, strerror(codigo));
            tlb_encerrar();
            return 0;
        }
        vigias_criadas++;
    }
    return 1;
}

int tlb_iniciar(Politica politica)
{
    int i;

    for (i = 0; i < ENTRADAS_TLB; i++) {
        tlb[i].valida = 0;
        tlb[i].pagina = -1;
        tlb[i].quadro = -1;
    }
    fila_iniciar(&fila_tlb, ENTRADAS_TLB, politica);
    return iniciar_vigias();
}

int tlb_buscar(int pagina, int *quadro)
{
    int resultado;

    pthread_mutex_lock(&painel.trava);
    painel.alvo = pagina;
    painel.achou = -1;
    painel.faltam = ENTRADAS_TLB;
    painel.rodada++;
    pthread_cond_broadcast(&painel.largada);

    while (painel.faltam > 0)
        pthread_cond_wait(&painel.chegada, &painel.trava);

    resultado = painel.achou;
    pthread_mutex_unlock(&painel.trava);

    if (resultado >= 0) {
        *quadro = tlb[resultado].quadro;
        fila_registrar_uso(&fila_tlb, resultado);
    }
    return resultado;
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
