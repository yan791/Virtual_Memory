#ifndef VM_CONFIG_H
#define VM_CONFIG_H
#define BITS_DESLOCAMENTO  8
#define TAM_PAGINA         (1 << BITS_DESLOCAMENTO)        
#define NUM_PAGINAS        256
#define NUM_QUADROS        128
#define ENTRADAS_TLB       16
#define TAM_ESPACO_VIRTUAL (NUM_PAGINAS * TAM_PAGINA)      
#define MAIOR_ENDERECO     (TAM_ESPACO_VIRTUAL - 1)        
#define MAIOR_FILA NUM_QUADROS
#define ARQUIVO_SAIDA   "correct.txt"
#define ARQUIVO_RESERVA "BACKING_STORE.bin"
#endif
