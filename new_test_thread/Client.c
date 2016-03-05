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

int isleader = 0;

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
	if ((write(socket_descriptor, mesg, strlen(mesg))) <= 0) {
		perror("erreur : impossible d'ecrire le message destine au serveur.");
		exit(1);
	}
}

char * crea_phrase(char * mot, char * code) {
    char * fin = malloc((strlen(mot) + strlen(code)) * sizeof(char));
    memcpy(fin, code, strlen(code));
    memcpy(fin + strlen(code), "~", 1);
    memcpy(fin + strlen(code) + 1, mot, strlen(mot)-1);
    return fin;
}

/////////////////////////////////////////////////////////////////////////////////////
/* complete la phrase a trou */
void complete_sentence(int socket_descriptor, char * phrase) {
    int i = 0;
    char * snippet = malloc(50* sizeof(char));
    char * oldReponse = malloc(strlen(phrase)* sizeof(char));
    char * tamponDeb = malloc(200* sizeof(char));
    char * tamponFin = malloc(200* sizeof(char));
    char * reponse = malloc(200* sizeof(char));
    printf("le leader vous demande de completer cette phrase: %s\n", phrase);
    while(i == 0){
        oldReponse = phrase;
        printf("par quoi voulez-vous remplacer le blanc (_) dans %s?\n", phrase);
        fgets (snippet, 50, stdin);
        printf("1 %s\n",snippet);

        tamponDeb = strtok(oldReponse,"_");
        printf("2 %s\n",tamponDeb);
        memcpy(reponse, tamponDeb, strlen(tamponDeb));
        printf("3 %s\n",reponse);
        memcpy(reponse + strlen(tamponDeb), snippet, strlen(snippet));
        printf("4 %s\n",reponse);
        tamponFin = strtok(NULL,"_");
        printf("2 %s\n",tamponFin);
        memcpy(reponse + strlen(tamponDeb) + strlen(snippet), tamponFin, strlen(tamponFin));

        printf("la phrase %s vous convient-elle? oui/non\n", reponse);
        fgets (snippet, 50, stdin);
        if(snippet == "oui") {
            i = 1;
        }
    }

    char * str = malloc( (strlen(reponse) + 5) *sizeof(char));
    str = crea_phrase(reponse,"0002");

    write_server(socket_descriptor,str);
}

/////////////////////////////////////////////////////////////////////////////////////
/* genere la phrase a trou avec verification de validité */
void write_sentence(int socket_descriptor) {
    printf("vous etes leader, quelle phrase envoyer? (format XXX_XX) : ");
    char* mesg = malloc(200*sizeof(char));
    fgets (mesg, 195, stdin);
    while ((strstr(mesg, "_") == NULL) && (strstr(mesg, "~") == NULL)) {
        printf("le message doit contenir le mot _ pour signifier la partie a completer\n");
        printf("il ne peut pas contenir le charactere ~ ni _\n");
        fgets (mesg, 195, stdin);
    }

    char * str = malloc( (strlen(mesg) + 5) *sizeof(char));
    str = crea_phrase(mesg,"0001");

    write_server(socket_descriptor,str);
}

/////////////////////////////////////////////////////////////////////////////////////
/* conversion code to int */
int convert_code(char * s) {
    return (((s[0] - '0')*1000) + ((s[1] - '0')*100) + ((s[2] - '0')*10) + ((s[3] - '0')));
}

/////////////////////////////////////////////////////////////////////////////////////
/* reponse au serveur qui demande une phrase a trou */
void reaction_message(int socket_descriptor, char * message) {

        char * code;
        char * phrase;
        char * reponse;
        code = malloc(5*sizeof(char));
        code = strtok(message,"~");
        // printf("%s|\n",code);
        phrase = malloc(256*sizeof(char));
        phrase = strtok(NULL,"~");
        // printf("%s|\n",phrase);

        //reaction
        switch(convert_code(code)){
            case 0 :
                printf("wat");
                break;
            case 1 :
                complete_sentence(socket_descriptor , phrase);
                break;
            case 2 :
                printf("phrase reçu: %s\n",phrase);
                break;
            case 3 :

                write_sentence(socket_descriptor);
                break;

        }
        return;

}


/////////////////////////////////////////////////////////////////////////////////////
/* lecture de la reponse en provenance du serveur */
// static void read_server(int socket_descriptor, char* buffer) {
// 	int longueur; /* longueur d'un buffer utilisé */   

// 	longueur = read(socket_descriptor, buffer, sizeof(buffer));
// 	buffer[longueur] = '\0';
// 	printf("\nreponse du serveur : ");
// 	write(1,buffer,longueur);
// }

void vie_client(int socket_descriptor) {
    /* initialisation de la file d'ecoute */
    //listen(socket_descriptor,5);
    char buffer[250];
    int longueur; 
    int i;
   
    if ((longueur = read(socket_descriptor, buffer, sizeof(buffer))) <= 0){
        printf("ayy");
        return ;
    } else {
        printf("avant le clean: %d : %s|\n", longueur, buffer);
        //traitement du message
        for(i = 0; i < longueur; i++) {
            if(buffer[i] == '\n') {
                buffer[i] = '\0';
            }
        }
        printf("reception d'un message de taille %d : %s|\n", longueur, buffer);

        reaction_message(socket_descriptor, buffer);

        return ;
    }

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
    char * temp = malloc(100*sizeof(char));
    size_t len = 0;
    printf("\n-----------------------------------------\n");
    printf("\nEcrivez votre pseudo : ");
    getline(&temp, &len, stdin);

    // strcpy(pseudo, "0000~");
    // printf("\n%s", pseudo);
    // strcat(pseudo, temp);
    // printf("\n%s", pseudo);
    // strcat(pseudo, "\n");
    // printf("\n%s", pseudo);

    char * code = malloc(5*sizeof(char));
    code = "0000";
    pseudo = crea_phrase(temp,code);
    printf("%s|\n",pseudo);
    write_server(socket_descriptor, pseudo);

    for(;;) {
        printf("\n-----------------------------------------\n");
    	printf("envoi d'un message au serveur : ");
    
    	/* ecrit un message de taille 100 max depuis la console */
    	//fgets (mesg, 100, stdin);
    		

        //////////////////////////////////////////////////////////////////
		/* envoi du message vers le serveur */
		// write_server(socket_descriptor, mesg);
		 
		// printf("message envoye au serveur.\n");
		
	 //    /* lecture de la reponse en provenance du serveur */        
		// //read_server(socket_descriptor, buffer);
		
		// printf("fin de la reception.\n");
        //////////////////////////////////////////////////////////////////
        vie_client(socket_descriptor);
    }
    
    close(socket_descriptor);
    printf("connexion avec le serveur fermee, fin du programme.\n");
    exit(0);
    
}
