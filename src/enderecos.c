#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "config.h"
#include "enderecos.h"
#include "erros.h"

static int lista_anexar(ListaEnderecos *lista, unsigned int endereco)
{
    if (lista->quantidade == lista->capacidade) {
        size_t nova = lista->capacidade ? lista->capacidade * 2 : 1024;
        unsigned int *maior;

        if (nova > (size_t)-1 / sizeof *maior)
            return 0;
        maior = realloc(lista->itens, nova * sizeof *maior);
        if (maior == NULL)
            return 0;
        lista->itens = maior;
        lista->capacidade = nova;
    }
    lista->itens[lista->quantidade++] = endereco;
    return 1;
}

static void descrever_caractere(int c, char *destino, size_t tamanho)
{
    if (c > 32 && c < 127)
        snprintf(destino, tamanho, "'%c'", c);
    else
        snprintf(destino, tamanho, "de codigo %d", c);
}


int carregar_enderecos(const char *caminho, ListaEnderecos *lista)
{
    enum { ANTES_DO_NUMERO, DENTRO_DO_NUMERO, DEPOIS_DO_NUMERO } estado;
    FILE *arquivo;
    struct stat info;
    unsigned long linha = 1;
    unsigned long primeira_linha_vazia = 0;
    unsigned long valor = 0;
    char descricao[32];
    int c;
    int ok = 0;

    arquivo = fopen(caminho, "r");
    if (arquivo == NULL) {
        erro("nao foi possivel abrir o arquivo de enderecos '%s': %s",
             caminho, strerror(errno));
        return 0;
    }
    if (fstat(fileno(arquivo), &info) == 0 && S_ISDIR(info.st_mode)) {
        erro("'%s' e um diretorio, nao um arquivo de enderecos", caminho);
        fclose(arquivo);
        return 0;
    }

    estado = ANTES_DO_NUMERO;
    for (;;) {
        c = getc(arquivo);

        if (c == EOF || c == '\n') {
            if (estado == ANTES_DO_NUMERO) {
                if (c == '\n' && primeira_linha_vazia == 0)
                    primeira_linha_vazia = linha;
            } else {
                if (primeira_linha_vazia != 0) {
                    erro("arquivo '%s', linha %lu: linha vazia no meio do "
                         "arquivo (esperado um endereco por linha)",
                         caminho, primeira_linha_vazia);
                    goto fim;
                }
                if (!lista_anexar(lista, (unsigned int)valor)) {
                    erro("memoria insuficiente para guardar os enderecos");
                    goto fim;
                }
            }
            if (c == EOF)
                break;
            estado = ANTES_DO_NUMERO;
            valor = 0;
            linha++;
        } else if (c == ' ' || c == '\t' || c == '\r') {
            if (estado == DENTRO_DO_NUMERO)
                estado = DEPOIS_DO_NUMERO;
        } else if (c >= '0' && c <= '9') {
            if (estado == DEPOIS_DO_NUMERO) {
                erro("arquivo '%s', linha %lu: mais de um valor na mesma linha",
                     caminho, linha);
                goto fim;
            }
            estado = DENTRO_DO_NUMERO;
            valor = valor * 10 + (unsigned long)(c - '0');
            if (valor > MAIOR_ENDERECO) {
                erro("arquivo '%s', linha %lu: endereco fora do intervalo "
                     "permitido (0 a %d)", caminho, linha, MAIOR_ENDERECO);
                goto fim;
            }
        } else {
            descrever_caractere(c, descricao, sizeof descricao);
            erro("arquivo '%s', linha %lu: caractere invalido %s (esperado "
                 "apenas um numero inteiro nao negativo por linha)",
                 caminho, linha, descricao);
            goto fim;
        }
    }

    if (ferror(arquivo)) {
        erro("falha ao ler o arquivo de enderecos '%s'", caminho);
        goto fim;
    }
    if (lista->quantidade == 0) {
        erro("o arquivo de enderecos '%s' nao contem nenhum endereco", caminho);
        goto fim;
    }
    ok = 1;

fim:
    fclose(arquivo);
    return ok;
}

void liberar_enderecos(ListaEnderecos *lista)
{
    free(lista->itens);
    lista->itens = NULL;
    lista->quantidade = 0;
    lista->capacidade = 0;
}
