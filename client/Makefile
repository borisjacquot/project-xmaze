# Makefile de l'executable du client

CC = gcc
SOURCES = $(wildcard *.c)
OBJETS = $(SOURCES:.c=.o)
CFLAGS = -Wall -Wextra -Werror
CIBLE = client

# Cible principale

all: $(CIBLE)

# Dependances pour l'executable

$(CIBLE): $(OBJETS)
	$(CC) -o $@ $^ 

# Nettoyage

clean: 
	rm -f core *.o $(CIBLE)