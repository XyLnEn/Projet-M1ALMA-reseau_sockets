# définition des actions
.PHONY: clean

# définition des variables
CFLAGS = -Wall -Wextra -pedantic -ansi
SOURCES = Array.c Serveur.c Client.c
OBJECTS = $(SOURCES:.c =.o)

# regles
all: Serveur Client

Serveur: 
	gcc Array.c Serveur.c -o Serveur.exe -lpthread
	chmod +x lancement_rapide.sh

Client: 
	gcc Client.c -o Client.exe


# action clean
clean:
	-rm *.o *~ Client.exe Serveur.exe
