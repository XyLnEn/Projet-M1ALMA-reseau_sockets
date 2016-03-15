/**
 * \file Client.c
 * \brief Client qui gère la partie en tant que joueur du jeu Sentence against humanity
 * \author Lenny Lucas - Alicia Boucard
 * \version 1
 * \date mars 2016
 *
 * Programme Client à lancer apres le serveur avec la commande : ./Client.exe
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <linux/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include "Array.h"


#define TAILLE_PHRASE_SANS_CODE 195
#define TAILLE_CODE 5
#define TAILLE_PHRASE_AVEC_CODE 200


typedef struct sockaddr 	sockaddr;
typedef struct sockaddr_in 	sockaddr_in;
typedef struct hostent 		hostent;
typedef struct servent 		servent;


/**
 * \struct listePhrases
 * \brief Structure composé d'une liste de phrases et de la taille de cette liste
 *
 * Structure représentant les phrases completées lors d'une partie.
 * La liste contient des objets de type chaine et connait le nombre d'objets qu'elle contient.
 */
typedef struct {
    char * tabPhrases[10];
    int nbPhrases;
} listePhrases;


/* Variable globale */
listePhrases tabReponses;
int isleader = 0;


/////////////////////////////////////////////////////////////////////////////////////
/**
 * \fn static void verif_arg(int argc)
 * \brief Fonction qui vérifie le nombre d'argument après l'exécutable
 *
 * \param argc int le nombre d'arguments reçu après l'éxécutable
 * \return void
 */
static void verif_arg(int argc) {
    if (argc != 1) {
        perror("Usage : ./client");
        exit(1);
    }
}


/////////////////////////////////////////////////////////////////////////////////////
/**
 * \fn static void find_ad_serv(hostent * ptr_host, char * host)
 * \brief Fonction qui trouve le serveur à partir de son adresse
 *
 * \param ptr_host pointeur qui stocke information sur une machine hote 
 * \param host chaine contenant le nom de la machine distante
 * \return void
 */
static void find_ad_serv(hostent * ptr_host, char * host) {
    if ((ptr_host = gethostbyname(host)) == NULL) {
        perror("erreur : impossible de trouver le serveur a partir de son adresse.");
        exit(1);
    }
}


/////////////////////////////////////////////////////////////////////////////////////
/**
 * \fn static int create_socket(int socket_descriptor)
 * \brief Fonction de création du socket
 *
 * \param socket_descriptor int qui stockera la description du socket
 * \return socket_descriptor la description du socket crée
 */
static int create_socket(int socket_descriptor) {
    if ((socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("erreur : impossible de creer la socket de connexion avec le serveur.");
            exit(1);
    }
    return socket_descriptor;
}


/////////////////////////////////////////////////////////////////////////////////////
/**
 * \fn static void connect_socket(int socket_descriptor, sockaddr_in adresse_locale)
 * \brief Fonction qui tente de se connecter au serveur dont les informations sont dans l'adresse
 *
 * \param socket_descriptor int qui stocke la description du socket
 * \param adresse_locale adresse de socket local de type sockaddr_in
 * \return void
 */
static void connect_socket(int socket_descriptor, sockaddr_in adresse_locale) {
    if ((connect(socket_descriptor, (sockaddr*)(&adresse_locale), sizeof(adresse_locale))) < 0) {
        perror("erreur : impossible de se connecter au serveur.");
        exit(1);
    }
    printf("_________________________________________\n");
    printf("|                                       |\n");
    printf("|       SENTENCE AGAINST HUMANITY       |\n");
    printf("|_______________________________________|\n");
    printf("\n-->Connexion établie avec le serveur.\n");
}


/////////////////////////////////////////////////////////////////////////////////////
/**
 * \fn static void write_server(int socket_descriptor, char *mesg)
 * \brief Fonction qui envoi un message vers le serveur
 *
 * \param socket_descriptor int stockant la description du socket
 * \param mesg chaine qui contient le message à envoyer
 * \return void
 */
static void write_server(int socket_descriptor, char *mesg) {
	if ((write(socket_descriptor, mesg, strlen(mesg))) <= 0) {
		perror("erreur : impossible d'ecrire le message destine au serveur.");
		exit(1);
	}
}


/////////////////////////////////////////////////////////////////////////////////////
/**
 * \fn char * crea_phrase(char * mot, char * code)
 * \brief Fonction génère la concaténation de code~ + message proprement
 *
 * \param mot chaine contenant le message envoyé par le joueur
 * \param code chaine contenant le code qui indique le type de message envoyé
 * \return fin la chaine contenant la concaténation de code~ + message
 */
char * crea_phrase(char * mot, char * code) {
    char * fin = malloc((strlen(mot) + strlen(code) + 1) * sizeof(char));
    memcpy(fin, code, strlen(code));
    memcpy(fin + strlen(code), "~", 1);
    printf("%zd : %s|\n",strlen(mot),mot);
    if(mot[strlen(mot)-1] == '\n') {
        memcpy(fin + strlen(code) + 1, mot, strlen(mot)-1);
    } else {
        memcpy(fin + strlen(code) + 1, mot, strlen(mot));
    }
    
    return fin;
}


/////////////////////////////////////////////////////////////////////////////////////
/**
 * \fn int convert_code(char * s)
 * \brief Fonction qui convertit un code en int
 *
 * \param s chaine contenant le code à convertir
 * \return le code converti en int
 */
int convert_code(char * s) {
    return (((s[0] - '0')*1000) + ((s[1] - '0')*100) + ((s[2] - '0')*10) + ((s[3] - '0')));
}


/////////////////////////////////////////////////////////////////////////////////////
/**
 * \fn void complete_sentence(int socket_descriptor, char * phrase)
 * \brief Fonction qui complète la phrase à trou en ajoutant la réponse du joueur
 *
 * \param socket_descriptor int stockant la description du socket
 * \param phrase chaine qui contient la phrase à tou
 * \return void
 */
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
        if(snippet[0] == 'o') {
            i = 1;
        }
    }
    char * str = malloc( (strlen(reponse) + TAILLE_CODE) *sizeof(char));
    str = crea_phrase(reponse,"0002");
    printf("%zd\n",strlen(str));
    write_server(socket_descriptor,str);
}


/////////////////////////////////////////////////////////////////////////////////////
/**
 * \fn void affiche_phrase(char * phrase)
 * \brief Fonction qui affiche une phrase
 *
 * \param phrase chaine qui contient une phrase
 * \return void
 */

void affiche_phrase(char * phrase) {
    if(!strstr(phrase,"|")) {
        printf("vous avez reçu la phrase: %s\n", phrase);
    }
}


/////////////////////////////////////////////////////////////////////////////////////
/**
 * \fn void choose_answer(int socket_descriptor)
 * \brief Fonction qui récupère la liste des phrases reçues et choisit le gagnant du tour de jeu
 *
 * \param socket_descriptor int stockant la description du socket
 * \return void
 */
void choose_answer(int socket_descriptor) {
    if(tabReponses.nbPhrases == 0) {
        printf("erreur: aucune phrases n'a été reçues.");
    } else {
        int i;
        char * number = malloc(TAILLE_CODE*sizeof(char));
        char * words = malloc(TAILLE_PHRASE_SANS_CODE*sizeof(char));
        char * index[10];
        for(i = 0; i < tabReponses.nbPhrases; i++) {
            index[i] = malloc(2*sizeof(char));
    
            strcpy(number, tabReponses.tabPhrases[i]);
            strcpy(index[i],strtok(number,"|"));
        
            words = strtok(NULL,"|");
            printf("phrase no %d : %s\n",i,words);
        }
        printf("quel est le gagnant? : ");
        fgets (number, 5, stdin);
        i = number[0] - '0';//transforme string en int
        char * str = malloc( (strlen(index[i]) + 5) *sizeof(char));
        str = crea_phrase(index[i],"0004");
        write_server(socket_descriptor,str);
    }
}


/////////////////////////////////////////////////////////////////////////////////////
/**
 * \fn void write_sentence(int socket_descriptor)
 * \brief Fonction qui génère la phrase à trou avec vérification de validité
 *
 * \param socket_descriptor int stockant la description du socket
 * \return void
 */
void write_sentence(int socket_descriptor) {
    printf("vous etes leader, quelle phrase envoyer? (format XXX_XX) : ");
    char* mesg = malloc(TAILLE_PHRASE_SANS_CODE*sizeof(char));
    char * clean = malloc((strlen(mesg) + 2) * sizeof(char));
    fgets (mesg, TAILLE_PHRASE_SANS_CODE, stdin);
    while ( (strlen(mesg) < 0) && (strstr(mesg, "_") == NULL) && (strstr(mesg, "~") == NULL)) {
        printf("le message doit contenir le mot _ pour signifier la partie a completer\n");
        printf("il ne peut pas contenir le charactere ~ ni _\n");
        fgets (mesg, TAILLE_PHRASE_SANS_CODE, stdin);
    }
    if(mesg[0] == '_') {
        memcpy(clean, " ", 1);
        memcpy(clean +1, mesg, strlen(mesg));
        memcpy(mesg, clean, strlen(clean));
    }
    if(mesg[strlen(mesg)-2] == '_') {
        memcpy(clean, mesg, strlen(mesg)-1);
        memcpy(clean + strlen(mesg)-1, " \0", 2);
        memcpy(mesg, clean, strlen(clean));
    }

    char * str = malloc( (strlen(mesg) + 5) *sizeof(char));
    str = crea_phrase(mesg,"0001");

    write_server(socket_descriptor,str);
}


/////////////////////////////////////////////////////////////////////////////////////
/**
 * \fn void reaction_message(int socket_descriptor, char * message)
 * \brief Fonction qui effectue l'action conrespondante en fonction du code du message
 *
 * \param socket_descriptor int stockant la description du socket
 * \param message chaine qui contient l'ensemble code + message
 * \return void
 */
void reaction_message(int socket_descriptor, char * message) {

        char * code;
        char * phrase;
        char * reponse;
        code = malloc(TAILLE_CODE*sizeof(char));
        code = strtok(message,"~");
        phrase = malloc(TAILLE_PHRASE_SANS_CODE*sizeof(char));
        phrase = strtok(NULL,"~");

        //reaction
        switch(convert_code(code)){
            case 0 ://uniquement pour serveur
                printf("wat");
                break;
            case 1 : //demande de completion de la phrase envoyée
                complete_sentence(socket_descriptor , phrase);
                break;
            case 2 : //reception de phrases completée, affichage simple
                if(isleader == 1) {
                    strcpy(tabReponses.tabPhrases[tabReponses.nbPhrases], phrase);
                    tabReponses.nbPhrases++;
                }
                affiche_phrase(phrase);
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
/**
 * \fn void vie_joueur(int socket_descriptor)
 * \brief Fonction qui écoute le serveur et qui réagit en fonction de se qu'il demande
 *
 * \param socket_descriptor int stockant la description du socket
 * \return void
 */
void vie_joueur(int socket_descriptor) {
    char buffer[TAILLE_PHRASE_AVEC_CODE];
    char * cleaned_sentence = malloc(TAILLE_PHRASE_AVEC_CODE * sizeof(char));
    strcpy(cleaned_sentence,"");
    int longueur; 
    int i;
   
    if ((longueur = read(socket_descriptor, buffer, sizeof(buffer))) <= 0){
        printf("ayy");
        return ;
    } else {

        //traitement du message
        for(i = 0; i < longueur; i++) {
            if(buffer[i] == '\n') {
                buffer[i] = '\0';
            }
        }
        memcpy(cleaned_sentence, buffer, longueur);
        cleaned_sentence[longueur+1] ='\0';

        reaction_message(socket_descriptor, cleaned_sentence);

        return ;
    }
}


/////////////////////////////////////////////////////////////////////////////////////
/**
 * \fn int main(int argc, char **argv)
 * \brief Entrée du programme.
 *
 * \param argc int nombre d'arguments
 * \param argv liste des arguments
 * \return EXIT
 */
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

    for(i=0; i<10; i++) {
        tabReponses.tabPhrases[i] = malloc(TAILLE_PHRASE_SANS_CODE*sizeof(char));
    }
    tabReponses.nbPhrases = 0;


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

    char * temp = malloc(TAILLE_PHRASE_SANS_CODE*sizeof(char));
    size_t len = 0;
    printf("\n-----------------------------------------\n");
    printf("\nEcrivez votre pseudo : ");
    getline(&temp, &len, stdin);

    char * code = malloc(TAILLE_CODE*sizeof(char));
    code = "0000";
    pseudo = crea_phrase(temp,code);
    write_server(socket_descriptor, pseudo);

    for(;;) {
        printf("\n-----------------------------------------\n");
        vie_joueur(socket_descriptor);
    }
    
    close(socket_descriptor);
    printf("connexion avec le serveur fermee, fin du programme.\n");
    exit(0);
    
}