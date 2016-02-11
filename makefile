# définition des actions
.PHONY: clean

# définition des variables
CC = gcc
#CFLAGS = -Wall -Wextra -pedantic
SOURCES = Array.c Serveur.c Client.c
OBJECTS = $(SOURCES:.c =.o)

# regles
serveur: Array.c Serveur.c
	$(CC) -o serveur Array.c Serveur.c

client: Client.c
	$(CC) -o client Client.c

Array.c: Array.h
	@touch Array.h

# action clean
clean:
	-rm *.o client serveur