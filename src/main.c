#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "enderecos.h"
#include "erros.h"
#include "fila_reposicao.h"
#include "memoria.h"
#include "tlb.h"

static void mostrar_uso(void)
{
    fputs("uso: ./vm <arquivo_de_enderecos> <fifo|lru> <fifo|lru>\n"
          "  1o argumento: arquivo com um endereco logico (0 a 65535) por linha\n"
          "  2o argumento: algoritmo de substituicao de pagina (fifo ou lru)\n"
          "  3o argumento: algoritmo de substituicao da TLB    (fifo ou lru)\n"
          "exemplo: ./vm address.txt lru fifo\n",
          stderr);
}

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
    ListaEnderecos enderecos = { NULL, 0, 0 };
    FILE *saida = NULL;
    unsigned long faltas = 0;
    unsigned long acertos_tlb = 0;
    size_t i;
    int status = EXIT_FAILURE;

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

    if (!carregar_enderecos(argv[1], &enderecos))
        goto liberar;

    if (!memoria_iniciar(politica_pagina))
        goto liberar;

    saida = fopen(ARQUIVO_SAIDA, "w");
    if (saida == NULL) {
        erro("nao foi possivel criar '%s': %s", ARQUIVO_SAIDA, strerror(errno));
        goto liberar;
    }

    if (!tlb_iniciar(politica_tlb))
        goto descartar_saida;

    for (i = 0; i < enderecos.quantidade; i++) {
        const unsigned int logico = enderecos.itens[i];
        const int pagina = (int)(logico >> BITS_DESLOCAMENTO);
        const int deslocamento = (int)(logico & (TAM_PAGINA - 1));
        int quadro = -1;
        int entrada = tlb_buscar(pagina, &quadro);
        int fisico;

        if (entrada >= 0) {                     
            acertos_tlb++;
        } else {                                
            quadro = memoria_consultar(pagina);
            if (quadro < 0) {                   
                int expulsa;

                faltas++;
                quadro = memoria_carregar(pagina, &expulsa);
                if (expulsa >= 0)
                    tlb_invalidar_pagina(expulsa);
                if (quadro < 0) {
                    erro("falha ao ler a pagina %d de '%s'",
                         pagina, ARQUIVO_RESERVA);
                    goto descartar_saida;
                }
            }
            entrada = tlb_inserir(pagina, quadro);
        }
        memoria_registrar_acesso(quadro);

        fisico = quadro * TAM_PAGINA + deslocamento;
        fprintf(saida, "Virtual address: %u TLB: %d Physical address: %d "
                       "Value: %d\n",
                logico, entrada, fisico, memoria_ler(fisico));
    }

    fprintf(saida, "Number of Translated Addresses = %lu\n",
            (unsigned long)enderecos.quantidade);
    fprintf(saida, "Page Faults = %lu\n", faltas);
    fprintf(saida, "Page Fault Rate = %.3f\n",
            (double)faltas / (double)enderecos.quantidade);
    fprintf(saida, "TLB Hits = %lu\n", acertos_tlb);
    fprintf(saida, "TLB Hit Rate = %.3f\n",
            (double)acertos_tlb / (double)enderecos.quantidade);

    tlb_encerrar();
    fclose(saida);
    saida = NULL;
    status = EXIT_SUCCESS;
    goto liberar;

descartar_saida:
    tlb_encerrar();
    fclose(saida);
    saida = NULL;
    remove(ARQUIVO_SAIDA);

liberar:
    tlb_encerrar();
    memoria_encerrar();
    liberar_enderecos(&enderecos);
    return status;
}
