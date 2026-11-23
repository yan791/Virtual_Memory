CFLAGS  = -std=c99 -O2 -Wall -Wextra -pedantic
FONTES     = $(wildcard src/*.c)
OBJETOS    = $(patsubst src/%.c,obj/%.o,$(FONTES))
CABECALHOS = $(wildcard include/*.h)

.PHONY: all clean

all: vm

vm: $(OBJETOS)
	$(CC) $(CFLAGS) -o $@ $(OBJETOS)

obj/%.o: src/%.c $(CABECALHOS) | obj
	$(CC) $(CFLAGS) -Iinclude -c $< -o $@

obj:
	mkdir -p obj

clean:
	rm -rf vm obj
