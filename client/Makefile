# Makefile de l'executable du client

CC = gcc
SOURCES = $(wildcard *.c)
OBJETS = $(SOURCES:.c=.o)
CFLAGS = -Wall -Wextra -Werror -I ../libs/libsockets -I ../common -I ../libs/libthreads
CIBLE = client
BIBLIOTHEQUES = -L ../libs/libsockets -lsockets -L ../libs/libthreads -lthreads -pthread

# Cible principale

all: $(CIBLE)

# Dependances pour l'executable

$(CIBLE): $(OBJETS)
	$(CC) -o $@ $^ $(BIBLIOTHEQUES)

# Nettoyage

clean: 
	rm -f core *.o $(CIBLE)
