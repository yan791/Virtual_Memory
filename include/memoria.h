#ifndef VM_MEMORIA_H
#define VM_MEMORIA_H
#include "fila_reposicao.h"

int memoria_iniciar(Politica politica);
void memoria_encerrar(void);
int memoria_consultar(int pagina);
int memoria_carregar(int pagina, int *pagina_expulsa);
void memoria_registrar_acesso(int quadro);
int memoria_ler(int endereco_fisico);

#endif
