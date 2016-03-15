# définition des actions
.PHONY: clean

# définition des variables
CC = gcc
CFLAGS = -Wall -Wextra -pedantic -ansi
SOURCES = Array.c Serveur.c Client.c
OBJECTS = $(SOURCES:.c =.o)

# regles
all: Serveur Client

Serveur: 
	$(CC) Array.c Serveur.c -o Serveur.exe -lpthread

Client: 
	$(CC) Client.c -o Client.exe

# action clean
clean:
	-rm *.o *~ Client.exe Serveur.exe
