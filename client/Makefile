# Makefile de l'executable du client

CC = gcc
SOURCES = $(wildcard *.c)
OBJETS = $(SOURCES:.c=.o)
CFLAGS = -Wall -Wextra -Werror -I ../libs/libsockets -I ../common -I ../libs/libthreads -I ../libs/Graphique
CIBLE = client
BIBLIOTHEQUES = -L ../libs/libsockets -lsockets -L ../libs/libthreads -lthreads -pthread -L ../libs/Graphique -lgraph -lSDL2_gfx -lSDL2

# Cible principale

all: $(CIBLE)

# Dependances pour l'executable

$(CIBLE): $(OBJETS)
	$(CC) -o $@ $^ $(BIBLIOTHEQUES)

# Nettoyage

clean: 
	rm -f core *.o $(CIBLE)
