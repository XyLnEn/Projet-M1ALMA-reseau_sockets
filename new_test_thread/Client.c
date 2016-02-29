/*-----------------------------------------------------------
Client a lancer apres le serveur avec la commande : ./Client.exe
------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <linux/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
// #include <pthread.h>
#include "Array.h"

typedef struct sockaddr 	sockaddr;
typedef struct sockaddr_in 	sockaddr_in;
typedef struct hostent 		hostent;
typedef struct servent 		servent;

/////////////////////////////////////////////////////////////////////////////////////
/* vérification du nombre d'argument */
static void verif_arg(int argc) {
    if (argc != 1) {
        perror("Usage : ./client");
        exit(1);
    }
}

/////////////////////////////////////////////////////////////////////////////////////
/* trouver le serveur à partir de son adresse */
static void find_ad_serv(hostent * ptr_host, char * host) {
    if ((ptr_host = gethostbyname(host)) == NULL) {
        perror("erreur : impossible de trouver le serveur a partir de son adresse.");
        exit(1);
    }
}

/////////////////////////////////////////////////////////////////////////////////////
/* creation de la socket */
static int create_socket(int socket_descriptor) {
    if ((socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("erreur : impossible de creer la socket de connexion avec le serveur.");
            exit(1);
    }
    return socket_descriptor;
}

/////////////////////////////////////////////////////////////////////////////////////
/* tentative de connexion au serveur dont les infos sont dans adresse_locale */
static void connect_socket(int socket_descriptor, sockaddr_in adresse_locale) {
    if ((connect(socket_descriptor, (sockaddr*)(&adresse_locale), sizeof(adresse_locale))) < 0) {
        perror("erreur : impossible de se connecter au serveur.");
        exit(1);
    }
    printf("_________________________________________\n");
    printf("|                                       |\n");
    printf("|       SENTENCE AGAINST HUMANITY       |\n");
    printf("|_______________________________________|\n");
    printf("\n-->Connexion etablie avec le serveur.\n");
}

/* envoi du message vers le serveur */
static void write_server(int socket_descriptor, const char *mesg) {
	if ((write(socket_descriptor, mesg, strlen(mesg))) < 0) {
		perror("erreur : impossible d'ecrire le message destine au serveur.");
		exit(1);
	}
}

/////////////////////////////////////////////////////////////////////////////////////
/* genere la phrase a trou avec verification de validité */
static void write_sentence(int socket_descriptor) {
    printf("quelle est la phrase que vous souhaiter envoyer?");
    char* mesg = NULL;
    fgets (mesg, 195, stdin);
    while ((strstr(mesg, "___") == NULL) && (strstr(mesg, "~") == NULL)) {
        printf("le message doit contenir le mot ___ pour signifier la partie a completer");
        printf("il ne peut pas contenir le charactere ~");
        mesg = NULL;
        fgets (mesg, 195, stdin);
    }

    char str[200];
    strcpy(str, "0001~");
    strcat(str, mesg);

    write_server(socket_descriptor,str);
}

/////////////////////////////////////////////////////////////////////////////////////
/* lecture de la reponse en provenance du serveur */
static void read_server(int socket_descriptor, char* buffer) {
	int longueur; /* longueur d'un buffer utilisé */   

	longueur = read(socket_descriptor, buffer, sizeof(buffer));
	buffer[longueur] = '\0';
	printf("\nreponse du serveur : ");
	write(1,buffer,longueur);
}

char * crea_pseudo(/*int socket_descriptor*/) {
    char* pseudo;
    char * temp;
    printf("\n-----------------------------------------\n");
    printf("\nEcrivez votre pseudo : ");
    fgets (temp, 30, stdin);
    strcpy(pseudo, "0000~");
    strcat(pseudo, temp);
    //write_server(socket_descriptor, str);
    return pseudo;
}

/////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv) {
    int socket_descriptor; 	    /* descripteur de socket */
    sockaddr_in adresse_locale; /* adresse de socket local */
    hostent * ptr_host;         /* info sur une machine hote */
    servent * ptr_service;      /* info sur service */
    char buffer[256];
    char * mesg;                /* message envoye */
    char * host = "LOCALHOST";  /* nom de la machine distante */
    char * pseudo;

    /* verification du nombre d'argument */
    verif_arg(argc);


    /* trouver le serveur à partir de son adresse */
    ///////////find_ad_serv(ptr_host, host);

    if ((ptr_host = gethostbyname(host)) == NULL) {
    perror("erreur : impossible de trouver le serveur a partir de son adresse.");
    exit(1);
    }
    
    /* copie caractere par caractere des infos de ptr_host vers adresse_locale */
    bcopy((char*)ptr_host->h_addr, (char*)&adresse_locale.sin_addr, ptr_host->h_length);
    adresse_locale.sin_family = AF_INET; /* ou ptr_host->h_addrtype; */
    
    /* 2 facons de definir le service que l'on va utiliser a distance */
    /* (commenter l'une ou l'autre des solutions) */
    
    /*-----------------------------------------------------------*/
    /* SOLUTION 1 : utiliser un service existant, par ex. "irc" */
    /*
    if ((ptr_service = getservbyname("irc","tcp")) == NULL) {
	perror("erreur : impossible de recuperer le numero de port du service desire.");
	exit(1);
    }
    adresse_locale.sin_port = htons(ptr_service->s_port);
    */
    /*-----------------------------------------------------------*/
    
    /*-----------------------------------------------------------*/
    /* SOLUTION 2 : utiliser un nouveau numero de port */
    adresse_locale.sin_port = htons(5000);
    /*-----------------------------------------------------------*/
    
   //printf("numero de port pour la connexion au serveur : %d \n", ntohs(adresse_locale.sin_port));
    
    /*-----------------------------------------------------------
    DEBUT DU JEU
    -----------------------------------------------------------*/

    /* creation de la socket */
    socket_descriptor = create_socket(socket_descriptor);
    
    /* tentative de connexion au serveur dont les infos sont dans adresse_locale */
    connect_socket(socket_descriptor, adresse_locale);

    //impossible de placer cela dans une fonction?!?! j'abandonne.
    char * temp;
    printf("\n-----------------------------------------\n");
    printf("\nEcrivez votre pseudo : ");
    fgets (temp, 30, stdin);
    strcpy(pseudo, "0000~");
    strcat(pseudo, temp);
    strcat(pseudo, "\n");
    printf("\n%s", pseudo);
    write_server(socket_descriptor, pseudo);

    for(;;) {
        printf("\n-----------------------------------------\n");
    	printf("envoi d'un message au serveur : ");
    
    	/* ecrit un message de taille 100 max depuis la console */
    	fgets (mesg, 100, stdin);
    		
		/* envoi du message vers le serveur */
		write_server(socket_descriptor, mesg);
		 
		printf("message envoye au serveur.\n");
		
	    /* lecture de la reponse en provenance du serveur */        
		//read_server(socket_descriptor, buffer);
		
		printf("fin de la reception.\n");	
    }
    
    close(socket_descriptor);
    printf("connexion avec le serveur fermee, fin du programme.\n");
    exit(0);
    
}
