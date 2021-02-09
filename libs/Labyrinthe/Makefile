#
# Makefile pour l'application de construction de labyrinthe
#

SOURCES = $(wildcard *.c)
OBJETS = $(SOURCES:.c=.o)
BIBLIOTHEQUES = -lm -L ../Graphique -lgraph -lSDL2_gfx -lSDL2
CFLAGS += -std=c99 -D_DEFAULT_SOURCE -I ../Graphique
CIBLE = labyrinthe

#
# Nom de la cible principale
#

all: $(CIBLE)

#
# Cible de nettoyage
#

clean: 
	rm -f core *.o $(CIBLE)

#
# Dependances pour l'executable
#

$(CIBLE): $(OBJETS)
	$(CC) -o $@ $^ $(BIBLIOTHEQUES)
