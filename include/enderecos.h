#ifndef VM_ENDERECOS_H
#define VM_ENDERECOS_H
#include <stddef.h>

typedef struct {
    unsigned int *itens;
    size_t quantidade;
    size_t capacidade;
} ListaEnderecos;
int carregar_enderecos(const char *caminho, ListaEnderecos *lista);
void liberar_enderecos(ListaEnderecos *lista);

#endif
