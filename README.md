# Gerenciador de Memória Virtual

Implementação em C do projeto *Designing a Virtual Memory Manager* (Silberschatz, *Operating System Concepts*, 10ª ed., p. P-51), com as modificações pedidas no enunciado:

- memória física com apenas **128 quadros**, preenchidos do 0 ao 127 antes de qualquer substituição;
- substituição de página **fifo** ou **lru**;
- substituição na TLB **fifo** ou **lru**;
- busca na TLB feita por **16 threads, uma para cada entrada**;
- saída no arquivo **`correct.txt`**, no mesmo formato do arquivo fornecido pelo professor;
- validação de todos os parâmetros, com mensagem de erro e código de saída 1.

## Parâmetros do sistema

| Item | Valor |
|---|---|
| Endereço lógico | 16 bits (8 de página + 8 de deslocamento) |
| Espaço virtual | 256 páginas de 256 bytes |
| Memória física | 128 quadros de 256 bytes (32 KB) |
| TLB | 16 entradas |
| Backing store | `BACKING_STORE.bin`, 65536 bytes |

## Requisitos

- Linux, Unix ou macOS
- compilador C (`gcc` ou `clang`) com suporte a POSIX threads
- `make`

## Compilação

```bash
make
```

Gera o executável `vm` na raiz do projeto (os objetos ficam em `obj/`). Para apagar o executável e os objetos:

```bash
make clean
```

O projeto é compilado com `-std=c99 -O2 -Wall -Wextra -pedantic -pthread`, sem warnings no gcc e no clang.

## Execução

```bash
./vm <arquivo_de_enderecos> <fifo|lru> <fifo|lru>
```

| Argumento | Significado |
|---|---|
| 1º | arquivo de endereços lógicos |
| 2º | algoritmo de substituição de página (`fifo` ou `lru`) |
| 3º | algoritmo de substituição da TLB (`fifo` ou `lru`) |

Exemplo, com lru na substituição de página e fifo na TLB:

```bash
./vm addresses.txt lru fifo
```

O `BACKING_STORE.bin` precisa estar no diretório atual. Quando tudo dá certo, nada é mostrado no terminal: o resultado é gravado no `correct.txt`, criado no diretório atual.

## Arquivo de endereços

Um número inteiro decimal por linha, de 0 a 65535, como no `addresses.txt`. São aceitos espaços e tabulações antes ou depois do número, quebras de linha do Windows (`\r\n`) e linhas em branco no final do arquivo. O arquivo inteiro é validado antes de qualquer tradução.

## Saída (`correct.txt`)

Uma linha por endereço traduzido, seguida das estatísticas:

```text
Virtual address: 16916 TLB: 0 Physical address: 20 Value: 0
Virtual address: 62493 TLB: 1 Physical address: 285 Value: 0
Virtual address: 30198 TLB: 2 Physical address: 758 Value: 29
...
Number of Translated Addresses = 1000
Page Faults = 539
Page Fault Rate = 0.539
TLB Hits = 54
TLB Hit Rate = 0.054
```

- `TLB`: índice da entrada da TLB usada na tradução (a que acertou ou a que recebeu a página);
- `Physical address`: quadro × 256 + deslocamento;
- `Value`: byte guardado nesse endereço físico, com sinal (-128 a 127).

## Resultados com o `addresses.txt`

| Página | TLB | Faltas de página | Acertos na TLB |
|---|---|---|---|
| fifo | fifo | 538 (0.538) | 54 (0.054) |
| fifo | lru | 538 (0.538) | 55 (0.055) |
| lru | fifo | 539 (0.539) | 54 (0.054) |
| lru | lru | 539 (0.539) | 55 (0.055) |

A execução `./vm addresses.txt lru fifo` gera um `correct.txt` idêntico, byte a byte, ao fornecido pelo professor.

## Tratamento de erros

Todos os erros mostram uma mensagem na saída de erro (stderr) e encerram o programa com código 1. Em todos os casos abaixo o programa para antes de criar o `correct.txt`, e um `correct.txt` já existente não é alterado.

| Situação | Mensagem |
|---|---|
| Número de argumentos errado | `vm: erro: numero de argumentos invalido: esperados 3, recebidos 0` (seguida do modo de uso) |
| Algoritmo de página inválido | `vm: erro: algoritmo de substituicao de pagina invalido: 'abc' (use fifo ou lru)` |
| Algoritmo da TLB inválido | `vm: erro: algoritmo de substituicao da TLB invalido: 'xyz' (use fifo ou lru)` |
| Arquivo de endereços inexistente | `vm: erro: nao foi possivel abrir o arquivo de enderecos 'nao_existe.txt': arquivo inexistente` |
| Diretório no lugar do arquivo | `vm: erro: 'src' e um diretorio, nao um arquivo de enderecos` |
| Letra, sinal ou ponto | `vm: erro: arquivo 'letras.txt', linha 1: caractere invalido 'a' (esperado apenas um numero inteiro nao negativo por linha)` |
| Valor acima de 65535 | `vm: erro: arquivo 'grande.txt', linha 1: endereco fora do intervalo permitido (0 a 65535)` |
| Dois valores na mesma linha | `vm: erro: arquivo 'dois.txt', linha 1: mais de um valor na mesma linha` |
| Linha vazia no meio do arquivo | `vm: erro: arquivo 'meio.txt', linha 2: linha vazia no meio do arquivo (esperado um endereco por linha)` |
| Arquivo vazio | `vm: erro: o arquivo de enderecos 'vazio.txt' nao contem nenhum endereco` |
| `BACKING_STORE.bin` ausente | `vm: erro: nao foi possivel abrir 'BACKING_STORE.bin' (ele deve estar no diretorio atual): arquivo inexistente` |
| `BACKING_STORE.bin` menor que 65536 bytes | `vm: erro: 'BACKING_STORE.bin' invalido: deve ser um arquivo com pelo menos 65536 bytes` |

## Estrutura do projeto

```text
.
├── Makefile
├── addresses.txt
├── BACKING_STORE.bin
├── evidencias.log
├── include/
│   ├── config.h           parâmetros do sistema e nomes dos arquivos
│   ├── enderecos.h
│   ├── erros.h
│   ├── fila_reposicao.h
│   ├── memoria.h
│   └── tlb.h
└── src/
    ├── main.c             argumentos, tradução dos endereços e correct.txt
    ├── enderecos.c        leitura e validação do arquivo de endereços
    ├── erros.c            mensagens de erro
    ├── fila_reposicao.c   fila usada pelo FIFO e pelo LRU
    ├── memoria.c          backing store, tabela de páginas e memória física
    └── tlb.c              TLB e as 16 threads de busca
```

## Como funciona

1. **Validação:** primeiro os argumentos, depois o arquivo de endereços inteiro (lido caractere por caractere) e, por fim, o `BACKING_STORE.bin`. Só então o `correct.txt` é criado.
2. **Tradução:** para cada endereço, página = endereço >> 8 e deslocamento = endereço & 0xFF. O programa procura a página na TLB; se não encontrar, consulta a tabela de páginas. Se a página não estiver na memória, ocorre uma falta de página: ela é lida do `BACKING_STORE.bin` para um quadro livre (de 0 a 127) ou para o quadro escolhido pelo algoritmo de substituição, e a tradução é inserida na TLB.
3. **FIFO e LRU:** uma única estrutura, a `FilaReposicao`, atende os dois algoritmos, tanto nos quadros quanto na TLB. É uma lista duplamente encadeada de índices em que a cabeça é sempre a próxima vítima. No FIFO, a ordem é a de chegada; no LRU, todo acesso, inclusive um acerto na TLB, leva o índice para o fim da fila.
4. **Invalidação:** quando uma página sai da memória, a entrada dela na TLB é invalidada, para que a TLB nunca devolva um quadro que já guarda outra página.
5. **Threads:** as 16 threads "vigias" são criadas uma vez, no início, e cada uma examina apenas a sua entrada da TLB. A cada busca, a thread principal publica a página procurada e acorda as vigias; cada vigia compara a sua entrada, e a última a responder acorda a thread principal. A sincronização usa um mutex e duas variáveis de condição (sem `pthread_barrier`, que não existe no macOS). No fim, as threads são encerradas com `pthread_join`.

## Testes rápidos

Compilar, executar e conferir as estatísticas:

```bash
make clean && make
./vm addresses.txt lru fifo
tail -5 correct.txt
```

Comparar com o `correct.txt` do professor (salvo com outro nome, por exemplo `correct_professor.txt`):

```bash
cmp correct.txt correct_professor.txt && echo IGUAIS
```

Caso limite da TLB: a página 0 é acessada entre cada uma das páginas 1 a 128, com fifo na página e lru na TLB. Ela continua na TLB, mas sai da memória quando a página 128 é carregada; como a entrada é invalidada, o último acesso à página 0 gera uma nova falta (130 faltas no total):

```bash
for i in $(seq 128); do echo 0; echo $((i*256)); done > /tmp/tlb_velha.txt
echo 0 >> /tmp/tlb_velha.txt
./vm /tmp/tlb_velha.txt fifo lru
tail -5 correct.txt
```

O registro dos testes de cada etapa do desenvolvimento está no `evidencias.log`.
