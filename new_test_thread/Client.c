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


#define TAILLE_PHRASE_SANS_CODE 195
#define TAILLE_CODE 5
#define TAILLE_PHRASE_AVEC_CODE 200

typedef struct {
    char * tabPhrases[10];
    int nbPhrases;
} listePhrases;



typedef struct sockaddr 	sockaddr;
typedef struct sockaddr_in 	sockaddr_in;
typedef struct hostent 		hostent;
typedef struct servent 		servent;


 //variable globales
listePhrases tabReponses;
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
static void write_server(int socket_descriptor, char *mesg) {
	if ((write(socket_descriptor, mesg, strlen(mesg))) <= 0) {
		perror("erreur : impossible d'ecrire le message destine au serveur.");
		exit(1);
	}
}

char * crea_phrase(char * mot, char * code) {
    char * fin = malloc((strlen(mot) + strlen(code)) * sizeof(char));
    memcpy(fin, code, strlen(code));
    memcpy(fin + strlen(code), "~", 1);
    memcpy(fin + strlen(code) + 1, mot, strlen(mot));
    return fin;
}

/////////////////////////////////////////////////////////////////////////////////////
/* conversion code to int */
int convert_code(char * s) {
    return (((s[0] - '0')*1000) + ((s[1] - '0')*100) + ((s[2] - '0')*10) + ((s[3] - '0')));
}

/////////////////////////////////////////////////////////////////////////////////////
/* complete la phrase a trou */
void complete_sentence(int socket_descriptor, char * phrase) {
    int i = 0;
    int j;
    char * snippet = malloc(50* sizeof(char));
    char * oldReponse = malloc(strlen(phrase)* sizeof(char));
    char * tamponDeb = malloc(200* sizeof(char));
    char * tamponFin = malloc(200* sizeof(char));
    char * reponse = malloc(TAILLE_PHRASE_SANS_CODE* sizeof(char));
    printf("le leader vous demande de completer cette phrase: %s\n", phrase);
    while(i == 0){
        memcpy(oldReponse, phrase, strlen(phrase));
        // oldReponse = phrase;
        printf("par quoi voulez-vous remplacer le blanc (_) dans %s?\n", phrase);
        fgets (snippet, 50, stdin);
        for(j = 0; j < strlen(snippet); j++) {
            if(snippet[j] == '\n') {
                snippet[j] = '\0';
            }
        }
        tamponDeb = strtok(oldReponse,"_");
        memcpy(reponse, tamponDeb, strlen(tamponDeb));

        memcpy(reponse + strlen(tamponDeb), snippet, strlen(snippet));
        tamponFin = strtok(NULL,"_");
        memcpy(reponse + strlen(tamponDeb) + strlen(snippet), tamponFin, strlen(tamponFin));

        printf("la phrase:\n %s \nvous convient-elle? oui/non\n", reponse);
        fgets (snippet, 50, stdin);
        if(snippet[0] == 'o') { //BUG
            i = 1;
        }
    }
    char * str = malloc( (strlen(reponse) + TAILLE_CODE) *sizeof(char));
    str = crea_phrase(reponse,"0002");
    write_server(socket_descriptor,str);
}


/////////////////////////////////////////////////////////////////////////////////////
/* recupere la liste des phrases reçu et choisit le gagnant */
void choose_answer(int socket_descriptor) {
    if(tabReponses.nbPhrases == 0) {
        printf("oups");
    } else {
        int i;
        char * number = malloc(TAILLE_CODE*sizeof(char));
        char * words = malloc(TAILLE_PHRASE_SANS_CODE*sizeof(char));
        char * index[10];
        for(i = 0; i < tabReponses.nbPhrases; i++) {
            index[i] = malloc(2*sizeof(char));
            printf("%s\n",tabReponses.tabPhrases[i]);
            strcpy(number, tabReponses.tabPhrases[i]);
            strcpy(index[i],strtok(number,"|"));
            // index[i] = strtok(number,"|");
            printf("%s\n",index[i]);
            words = strtok(NULL,"|");
            printf("phrase no %d : %s\n",i,words);
        }
        printf("quel est le gagnant? : ");
        fgets (number, 5, stdin);
        printf("---->%s\n",index[i]);
        i = number[0] - '0';//transforme string en int
        printf("---%d, ->%s\n",i,index[i]);
        char * str = malloc( (strlen(index[i]) + 5) *sizeof(char));
        str = crea_phrase(index[i],"0004");
        write_server(socket_descriptor,str);

    }
}

/////////////////////////////////////////////////////////////////////////////////////
/* genere la phrase a trou avec verification de validité */
void write_sentence(int socket_descriptor) {
    printf("vous etes leader, quelle phrase envoyer? (format XXX_XX) : ");
    char* mesg = malloc(TAILLE_PHRASE_SANS_CODE*sizeof(char));
    fgets (mesg, TAILLE_PHRASE_SANS_CODE, stdin);
    while ((strstr(mesg, "_") == NULL) && (strstr(mesg, "~") == NULL)) {
        printf("le message doit contenir le mot _ pour signifier la partie a completer\n");
        printf("il ne peut pas contenir le charactere ~ ni _\n");
        fgets (mesg, TAILLE_PHRASE_SANS_CODE, stdin);
    }

    char * str = malloc( (strlen(mesg) + 5) *sizeof(char));
    str = crea_phrase(mesg,"0001");

    write_server(socket_descriptor,str);
}



/////////////////////////////////////////////////////////////////////////////////////
/* reponse au serveur qui demande une phrase a trou */
void reaction_message(int socket_descriptor, char * message) {

        char * code;
        char * phrase;
        char * reponse;
        code = malloc(TAILLE_CODE*sizeof(char));
        code = strtok(message,"~");
        // printf("%s|\n",code);
        phrase = malloc(TAILLE_PHRASE_SANS_CODE*sizeof(char));
        phrase = strtok(NULL,"~");
        // printf("%s|\n",phrase);

        //reaction
        switch(convert_code(code)){
            case 0 ://uniquement pour serveur, OH OH PROBLEMO
                printf("wat");
                break;
            case 1 : //demande de completion de la phrase envoyée
                complete_sentence(socket_descriptor , phrase);
                break;
            case 2 : //reception de phrases completée, affichage simple
                if(isleader == 1) {
                    strcpy(tabReponses.tabPhrases[tabReponses.nbPhrases], phrase);
                    // tabReponses.tabPhrases[tabReponses.nbPhrases] = phrase;
                    tabReponses.nbPhrases++;
                }
                printf("phrase reçue: %s\n",phrase);
                break;
            case 3 : //demande speciale: transforme un non-leader en leader ou demande au leader de choisir une phrase de resultat
                if(isleader == 1) {
                    choose_answer(socket_descriptor);
                    isleader = 0;//une fois que la reponse est choisie, on n'est plus leader
                    tabReponses.nbPhrases = 0;
                } else {
                    isleader = 1;// deviens leader
                    write_sentence(socket_descriptor);
                }
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
    char buffer[TAILLE_PHRASE_AVEC_CODE];
    char * cleaned_sentence = malloc(TAILLE_PHRASE_AVEC_CODE * sizeof(char));
    strcpy(cleaned_sentence,"");
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
        memcpy(cleaned_sentence, buffer, longueur);


        printf("reception d'un message de taille %d : %s|\n", longueur, cleaned_sentence);

        reaction_message(socket_descriptor, cleaned_sentence);


        // printf("reception d'un message de taille %d : %s|\n", longueur, buffer);

        // reaction_message(socket_descriptor, buffer);

        return ;
    }

}



/////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv) {
    int i;

    int socket_descriptor; 	    /* descripteur de socket */
    sockaddr_in adresse_locale; /* adresse de socket local */
    hostent * ptr_host;         /* info sur une machine hote */
    servent * ptr_service;      /* info sur service */
    char buffer[TAILLE_PHRASE_AVEC_CODE];
    char * mesg;                /* message envoye */
    char * host = "LOCALHOST";  /* nom de la machine distante */
    char * pseudo;

    /* verification du nombre d'argument */
    verif_arg(argc);

    //init table des phrases
    // tabReponses.tabPhrases = malloc(10*sizeof(char*));
    for(i=0; i<10; i++) {
        tabReponses.tabPhrases[i] = malloc(TAILLE_PHRASE_SANS_CODE*sizeof(char));
    }
    tabReponses.nbPhrases = 0;


    ////////////////////////////////simulation///////////////////////////////
    /*
    printf("debug? o/n : ");
    char * abcdefgtemp = malloc(50*sizeof(char));
    fgets (abcdefgtemp, 50, stdin);
    if(abcdefgtemp[0] == 'o') {
        printf("phrase 'int|string': ");
        abcdefgtemp = malloc(50*sizeof(char));
        fgets (abcdefgtemp, 50, stdin);        
        tabReponses.tabPhrases[tabReponses.nbPhrases] = abcdefgtemp;
        tabReponses.nbPhrases++;
        isleader = 1;
    }

    */
    /////////////////////////////fin simulation//////////////////////////////


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
    char * temp = malloc(TAILLE_PHRASE_SANS_CODE*sizeof(char));
    size_t len = 0;
    printf("\n-----------------------------------------\n");
    printf("\nEcrivez votre pseudo : ");
    getline(&temp, &len, stdin);

    char * code = malloc(TAILLE_CODE*sizeof(char));
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
