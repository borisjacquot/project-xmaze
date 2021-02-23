# Makefile de l'executable du client

CC = gcc
SOURCES = $(wildcard *.c)
OBJETS = $(SOURCES:.c=.o)
CFLAGS = -Wall -Wextra -Werror -I ../libs/libsockets -I ../common
CIBLE = server
BIBLIOTHEQUES = -L ../libs/libsockets -lsockets


# Cible principale

all: $(CIBLE)

# Dependances pour l'executable

$(CIBLE): $(OBJETS)
	$(CC) -o $@ $^ $(BIBLIOTHEQUES)

# Nettoyage

clean: 
	rm -f core *.o $(CIBLE)
