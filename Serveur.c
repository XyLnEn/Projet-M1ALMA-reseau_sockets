/**
 * \file Serveur.c
 * \brief Serveur qui gère une partie en tant que maitre du jeu Sentence against humanity
 * \author Lenny Lucas - Alicia Boucard
 * \version 1
 * \date mars 2016
 *
 * Programme Serveur à lancer avant le Client avec la commande : ./Serveur.exe
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <linux/types.h>    /* pour les sockets */
#include <sys/socket.h>
#include <netdb.h>          /* pour hostent, servent */
#include <string.h> 
#include <pthread.h>        /* pour bcopy, ... */
#include "Array.h"


#define TAILLE_MAX_NOM 256
#define NB_CLIENTS_MAX 10
#define ATTENTE_DEBUT_PARTIE 10
#define ATTENTE_FIN_JEU 10
#define TAILLE_PHRASE_SANS_CODE 195
#define TAILLE_CODE 5
#define TAILLE_PHRASE_AVEC_CODE 200
#define SCORE_FIN_PARTIE 1


typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;
typedef struct hostent hostent;
typedef struct servent servent;

struct timeval timeout;


/**
 * \struct reponse_joueur
 * \brief Structure composé d'une chaine et d'un identifiant
 *
 * Structure représentant une réponse d'un joueur.
 * La chaine se termine obligatoirement par un zéro de fin et
 * connait l'identifiant du joueur qui a envoyé la réponse.
 */
typedef struct 
{
    int ident;
    char * phrase;
} 
reponse_joueur;


/**
 * \struct liste_reponse_joueurs
 * \brief Structure composé d'une liste de réponses et de la taille de cette liste
 *
 * Structure stockant les réponses des joueurs lors d'une partie.
 * La liste contient des objets de type reponse_joueur et connait le nombre d'objets qu'elle contient.
 */
typedef struct 
{
    reponse_joueur liste_rep[NB_CLIENTS_MAX];
    int nb_contenu;
}
liste_reponse_joueurs;


/**
 * \struct serveur_t
 * \brief Structure pour la gestion des threads et du serveur
 *
 * Structure stockant les informations des threads clients (joueurs) et du serveur.
 * Les threads partagent les informations sur les clients et la liste des réponses.
 */
typedef struct
{
   Array * tabJoueurs;
   liste_reponse_joueurs * tabReponses;
 
   pthread_t thread_serveur;
   pthread_t thread_joueurs [NB_CLIENTS_MAX];
   pthread_mutex_t mutex_stock;

   int fin_partie;
}
serveur_t;


/* Variable globale de type serveur_t */
serveur_t serveur;


/////////////////////////////////////////////////////////////////////////////////////
/**
 * \fn int create_socket(int socket_descriptor)
 * \brief Fonction de création du socket
 *
 * \param socket_descriptor int qui stockera la description du socket
 * \return socket_descriptor la description du socket crée
 */
int create_socket(int socket_descriptor) {
    if ((socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("erreur : impossible de creer la socket de connexion avec le client.");
        exit(1);
    }
}


/////////////////////////////////////////////////////////////////////////////////////
/**
 * \fn void bind_socket(int socket_descriptor, sockaddr_in adresse_locale)
 * \brief Fonction qui associe le socket à l'adresse de connexion
 *
 * \param socket_descriptor int stockant la description du socket
 * \param adresse_locale sockaddr_in est la structure d'adresse locale
 * \return void
 */
void bind_socket(int socket_descriptor, sockaddr_in adresse_locale) {
    if ((bind(socket_descriptor, (sockaddr*)(&adresse_locale), sizeof(adresse_locale))) < 0) {
        perror("erreur : impossible de lier la socket a l'adresse de connexion.");
        exit(1);
    }
}


/////////////////////////////////////////////////////////////////////////////////////
/**
 * \fn static void write_player(int socket_descriptor, char *mesg)
 * \brief Fonction qui envoi un message vers un joueur
 *
 * \param socket_descriptor int stockant la description du socket
 * \param mesg chaine qui contient le message à envoyer
 * \return void
 */
static void write_player(int socket_descriptor, char *mesg) {
    if ((write(socket_descriptor, mesg, strlen(mesg))) <= 0) {
        perror("erreur : impossible d'ecrire le message destine au joueur.");
        exit(1);
    }
}


/////////////////////////////////////////////////////////////////////////////////////
/**
 * \fn char * reception(int sock)
 * \brief Fonction qui reçoit un message envoyé par un joueur
 *
 * \param sock int stockant la description du socket
 * \return cleaned_sentence une chaine contenant le message envoyé par le joueur
 */
char * reception(int sock) {
    char * buffer = malloc(TAILLE_PHRASE_AVEC_CODE * sizeof(char));
    char * cleaned_sentence = malloc(TAILLE_PHRASE_AVEC_CODE * sizeof(char));
    strcpy(cleaned_sentence,"");
    int longueur;
   
    if ((longueur = read(sock, buffer, TAILLE_PHRASE_AVEC_CODE)) <= 0){ 
        return "";
    }

    // nettoie la fin du mot pour qu'il se termine obligatoirement par un zéro de fin 
    int i;
    for(i = 0; i < longueur; i++) {
        if(buffer[i] == '\n') {
            buffer[i] = '\0';
        }
    }

    memcpy(cleaned_sentence, buffer, longueur);
    cleaned_sentence[longueur+1] ='\0';

    printf("reception d'un message de taille %d : %s\n", longueur, cleaned_sentence);
    return cleaned_sentence;
}


////////////////////////////////////////////////////////////////////////////////////
/**
 * \fn char * crea_phrase(char * mot, char * code)
 * \brief Fonction génère la concaténation de code~ + message proprement
 *
 * \param mot chaine contenant le message envoyé par le joueur
 * \param code chaine contenant le code qui indique le type de message envoyé
 * \return fin la chaine contenant la concaténation de code~ + message
 */
char * crea_phrase(char * mot, char * code) {

    char * fin = malloc((strlen(mot) + strlen(code)+1) * sizeof(char));
    memcpy(fin, code, strlen(code));
    memcpy(fin + strlen(code), "~", 1);
    memcpy(fin + strlen(code) + 1, mot, strlen(mot));
    return fin;
}


/////////////////////////////////////////////////////////////////////////////////////
/**
 * \fn void prevenir_leader()
 * \brief Fonction qui informe au joueur qu'il est leader
 *
 * \return void
 */
void prevenir_leader() {
    char * trigger = malloc(7*sizeof(char));
    trigger = crea_phrase("go","0003");
    int i = 0;
    for (i = 0; i < serveur.tabJoueurs->used; ++i) {
       if(serveur.tabJoueurs->array[i].leader == 1) {
            write_player(serveur.tabJoueurs->array[i].socket,trigger);

       } 
    }
}


/////////////////////////////////////////////////////////////////////////////////////
/**
 * \fn void affich_joueurs()
 * \brief Fonction qui affiche la liste des joueurs de la partie
 *
 * \return void
 */
void affich_joueurs() {
    printf("*******************************************\n");
    printf("* pseudo\t * score\t * leader *\n");
    printf("*******************************************\n");
    int i = 0;
    for (i = 0; i < serveur.tabJoueurs->used; ++i) {
        printf("* %s\t\t * %d\t\t * %d\t *\n", 
             serveur.tabJoueurs->array[i].pseudo, serveur.tabJoueurs->array[i].score,
             serveur.tabJoueurs->array[i].leader);
    }
    printf("*******************************************\n");
    return;
}


/////////////////////////////////////////////////////////////////////////////////////
/**
 * \fn void choix_leader()
 * \brief Fonction qui attribue le statut de leader à un joueur
 * \return void
 */
void choix_leader() {
    if(serveur.tabJoueurs->used == 0) {
        perror("erreur : choix leader impossible");
        exit(1);
    } else {
        if(serveur.tabJoueurs->used == 1) {
            serveur.tabJoueurs->array[0].leader = 1;
        } else if(serveur.tabJoueurs->array[serveur.tabJoueurs->used-1].leader == 1) {
            serveur.tabJoueurs->array[serveur.tabJoueurs->used-1].leader = 0;
            serveur.tabJoueurs->array[0].leader = 1;
        }else {
            int i = 0;
            int j = 0;
            for (i = 0; i < serveur.tabJoueurs->used - 1; ++i) {
                if(serveur.tabJoueurs->array[i].leader == 1) {
                    j = 1;
                    serveur.tabJoueurs->array[i].leader = 0;
                    serveur.tabJoueurs->array[i+1].leader = 1;
                    i = serveur.tabJoueurs->used;
                } 
            }
            if(j == 0) { //pas trouvé de leader
                serveur.tabJoueurs->array[0].leader = 1;
            }
        }
        affich_joueurs();
        prevenir_leader();
    }
    return;
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


/////////////////////////////////////////////////////////////////////////////////
/**
 * \fn void prevenir_joueurs(int k, char * suiv)
 * \brief Fonction prévient les clients du gagnant du tour
 *
 * \param k int indice du gagnant
 * \param suiv chaine contenant une notification envoyée aux joueurs
 * \return void
 */
void prevenir_joueurs(int k, char * suiv) {
    char * reponse = malloc(TAILLE_PHRASE_AVEC_CODE * sizeof(char));
    int w;
    memcpy(reponse, serveur.tabJoueurs->array[k].pseudo, strlen(serveur.tabJoueurs->array[k].pseudo));
    memcpy(reponse + strlen(serveur.tabJoueurs->array[k].pseudo) , suiv, strlen(suiv));
    reponse = crea_phrase(reponse,"0002");
    for(w = 0; w < serveur.tabJoueurs->used; w++) {
        write_player(serveur.tabJoueurs->array[w].socket,reponse);
        sleep(1);
    }
}


/////////////////////////////////////////////////////////////////////////////////////
/**
 * \fn void decode(char * test, int nouv_socket_descriptor, Array * tabJoueurs)
 * \brief Fonction qui évalue le code et effectue les actions correspondantes 
 *
 * \param test chaine contenant le message à décoder
 * \param nouv_socket_descriptor int stockant la description du socket
 * \param tabJoueurs tableau de joueurs
 * \return void
 */
void decode(char * test, int nouv_socket_descriptor, Array * tabJoueurs) {

    if (strstr(test,"~")) {
        Info_player element;
        int i;
        int j;
        int k;

        char * code;
        char * phrase;

        code = malloc(TAILLE_CODE*sizeof(char));
        strcpy(code,"");
        code = strtok(test,"~");

        phrase = malloc(TAILLE_PHRASE_SANS_CODE*sizeof(char));
        strcpy(phrase,"");
        phrase = strtok(NULL,"~");
 
        char * reponse;
        reponse = malloc(TAILLE_PHRASE_AVEC_CODE * sizeof(char));
        strcpy(reponse,"");
        i = convert_code(code);

        if (i == 0){
            element.socket = nouv_socket_descriptor;
            
            element.pseudo = malloc(TAILLE_PHRASE_SANS_CODE*sizeof(char));
            strcpy(element.pseudo,phrase);

            element.score = 0;
            element.leader = 0;
            insertArray(serveur.tabJoueurs, element);
        
            return;
        }
        else if (i == 1) {
            reponse = crea_phrase(phrase,"0001");

            for (j = 0; j < serveur.tabJoueurs->used; ++j) {
                if(serveur.tabJoueurs->array[j].leader == 0) {
                    write_player(serveur.tabJoueurs->array[j].socket,reponse);
                } 
            }
        }
        else if (i == 2) {
            pthread_mutex_lock (& serveur.mutex_stock);
         
            if(serveur.fin_partie == 0) {
                serveur.fin_partie = 1;
            }

            //ecriture du couple phrase/numero dans le tab de reponses.
            reponse_joueur * rep = malloc(sizeof(reponse_joueur));
            rep->ident = nouv_socket_descriptor;

            rep->phrase = malloc(strlen(phrase) * sizeof(char) );
            strcpy(rep->phrase,phrase);

            serveur.tabReponses->liste_rep[serveur.tabReponses->nb_contenu] = *rep;
            serveur.tabReponses->nb_contenu++;

            pthread_mutex_unlock (& serveur.mutex_stock);
        }
        else if (i == 4) {
            j = phrase[0] - '0';
            for(k = 0; k < serveur.tabJoueurs->used; k++) {
                if(serveur.tabJoueurs->array[k].socket == j) { //trouvé le gagnant !
                    serveur.tabJoueurs->array[k].score++;
                    if(serveur.tabJoueurs->array[k].score >= SCORE_FIN_PARTIE) {
                        strcpy(reponse, " a gagne la partie, CONGRATULATION!");
                        printf("%s est le gagnant! CONGRATULATION!\n",serveur.tabJoueurs->array[k].pseudo);
                        prevenir_joueurs(k,reponse);
                        printf("nouvelle partie? oui/non\n");
                        fgets (reponse, 50, stdin);
                        if(reponse[0] == 'o') {
                            for(k = 0; k < serveur.tabJoueurs->used; k++) {
                                serveur.tabJoueurs->array[k].score = 0;
                            }
                            choix_leader();

                        } else {
                            for(k = 0; k < serveur.tabJoueurs->used; k++) {
                                write_player(serveur.tabJoueurs->array[k].socket,crea_phrase("bye","0000"));
                            }
                            exit(0);
                        }
                    } else {
                        strcpy(reponse, " a gagne ce tour!");
                        printf("%s remporte le point!\n",serveur.tabJoueurs->array[k].pseudo);
                        serveur.tabReponses->nb_contenu = 0;
                        prevenir_joueurs(k,reponse);
                        choix_leader();
                    }
                }
            }
        }
    }
    return;
}


/////////////////////////////////////////////////////////////////////////////////////
/**
 * \fn int accept_client(int socket_descriptor, sockaddr_in adresse_client_courant, int longueur_adresse_courante)
 * \brief Fonction qui connexion un nouveau joueur
 *
 * \param socket_descriptor int stockant la description du socket
 * \param adresse_client_courant sockaddr_in adresse client courant
 * \param longueur_adresse_courante int longueur d'adresse courante d'un client
 * \return nouv_socket_descriptor stockant la description du socket
 */
int accept_client(int socket_descriptor, sockaddr_in adresse_client_courant, int longueur_adresse_courante) {

    int nouv_socket_descriptor;
    if ((nouv_socket_descriptor = accept(socket_descriptor, 
      (sockaddr*)(&adresse_client_courant), &longueur_adresse_courante)) < 0) {
        perror("erreur : impossible d'accepter la connexion avec le client.");
        exit(1);
    } 
    return nouv_socket_descriptor;
}


/////////////////////////////////////////////////////////////////////////////////////
/**
 * \fn void envoi_resultat_leader()
 * \brief Fonction envoie les phrases complétées au leader
 *
 * \return void
 */
void envoi_resultat_leader() {

    char * reponse; 
    char * phrase;
    char nb[5];
    int i;
    int j;
    for (i = 0; i < serveur.tabJoueurs->used; ++i) {
        if(serveur.tabJoueurs->array[i].leader == 1) {
            j = serveur.tabJoueurs->array[i].socket;
            i = serveur.tabJoueurs->used;
        } 
    }
    for (i = 0; i < serveur.tabReponses->nb_contenu; ++i)
    {
        reponse = malloc((TAILLE_PHRASE_AVEC_CODE + 3)*sizeof(char));
        phrase = malloc (TAILLE_PHRASE_SANS_CODE * sizeof(char));

        sprintf(nb, "%d", serveur.tabReponses->liste_rep[i].ident);
        memcpy(phrase, nb, strlen(nb));
        memcpy(phrase + strlen(nb), "|", 1);
        memcpy(phrase + strlen(nb) + 1, serveur.tabReponses->liste_rep[i].phrase , strlen(serveur.tabReponses->liste_rep[i].phrase));
        
        reponse = crea_phrase(phrase,"0002");

        printf("%d, %s de taille %zd\n",i,reponse, strlen(reponse));
        
        write_player(j, reponse);
        sleep(1);
    }
    write_player(j, "0003~go");
 
    serveur.fin_partie = 0;
    return;
}


/////////////////////////////////////////////////////////////////////////////////////
/**
 * \fn static void * mj_main (void * p_data)
 * \brief Fonction qui encapsule le comportement du serveur
 *
 * \param p_data pointeur vers une donnée quelconque
 * \return void *
 */
static void * mj_main (void * p_data) {

    initArray(serveur.tabJoueurs, 5);
    sleep (ATTENTE_DEBUT_PARTIE);

    while(serveur.tabJoueurs->used < 3) {
        sleep(1);
    }
    printf("il y a %zd participants pour ce tour, debut...\n",serveur.tabJoueurs->used);
    choix_leader();
    while (1) {
         /* Debut de la zone protegee. */
      pthread_mutex_lock (& serveur.mutex_stock);
      if(serveur.fin_partie == 1) {//on a reçu au moins 1 reponse, debut du compte a rebour
        pthread_mutex_unlock (& serveur.mutex_stock);

        sleep(ATTENTE_FIN_JEU);

        pthread_mutex_lock (& serveur.mutex_stock);

        printf("fin du jeu, choix du gagnant: \n");

        envoi_resultat_leader();
      }
      pthread_mutex_unlock (& serveur.mutex_stock);
    }
    return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////
/**
 * \fn static void * joueur_main (void * p_data)
 * \brief Fonction qui encapsule la liaison entre le serveur et un joueur
 *
 * \param p_data pointeur vers une donnée quelconque
 * \return void *
 */
static void * joueur_main (void * p_data)
{
    char * pseudo;
    char * temp;
    int nouv_socket_descriptor = (intptr_t) p_data;
 
   while (1)
   {
        pseudo = reception(nouv_socket_descriptor);
        decode(pseudo,nouv_socket_descriptor, serveur.tabJoueurs);
   }
 
   return NULL;
}


/////////////////////////////////////////////////////////////////////////////////////
/**
 * \fn int main (void)
 * \brief Entrée du programme.
 *
 * \return EXIT_SUCCESS
 */
int main (void)
{
    serveur.tabJoueurs = malloc(sizeof(Array));
    serveur.tabReponses = malloc(sizeof(liste_reponse_joueurs));
    serveur.mutex_stock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;

    int socket_descriptor,          /* descripteur de socket */
        nouv_socket_descriptor,     /* [nouveau] descripteur de socket */
        longueur_adresse_courante;  /* longueur d'adresse courante d'un client */
    sockaddr_in adresse_locale,     /* structure d'adresse locale*/
        adresse_client_courant;     /* adresse client courant */
    hostent * ptr_hote;             /* les infos recuperees sur la machine hote */
    servent * ptr_service;          /* les infos recuperees sur le service de la machine */
    char machine[TAILLE_MAX_NOM+1]; /* nom de la machine locale */
    char * pseudo = "";

    int i = 0;
    int thread_Maitre_Jeu = 0;
    int thread_Liaison_Joueur = 0;
    serveur.fin_partie = 0;
 
    /* Creation du thread du serveur. */
    printf ("Creation du thread du serveur !\n");
    thread_Maitre_Jeu = pthread_create (
        & serveur.thread_serveur, NULL, mj_main, NULL);

//----------------------------------------------------------------------------------------------------------
    gethostname(machine,TAILLE_MAX_NOM); /* recuperation du nom de la machine */
    
    /* recuperation de la structure d'adresse en utilisant le nom */
    //find_ad_serv(ptr_hote, machine);
    if ((ptr_hote = gethostbyname(machine)) == NULL) {
        perror("erreur : impossible de trouver le serveur a partir de son adresse.");
        exit(1);
    }
    
    /* initialisation de la structure adresse_locale avec les infos recuperees */           
    
    /* copie de ptr_hote vers adresse_locale */
    bcopy((char*)ptr_hote->h_addr, (char*)&adresse_locale.sin_addr, ptr_hote->h_length);
    adresse_locale.sin_family       = ptr_hote->h_addrtype;     /* ou AF_INET */
    adresse_locale.sin_addr.s_addr  = INADDR_ANY;           /* ou AF_INET */

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
    /* SOLUTION 2 : utiliser un nouveau numero de port */
    adresse_locale.sin_port = htons(5000);
    /*-----------------------------------------------------------*/
    
    printf("\n|-----------------------------------------------------|\n");
    printf("|-->Numero de port pour la connexion au serveur : %d|", 
           ntohs(adresse_locale.sin_port) /*ntohs(ptr_service->s_port)*/);
    printf("\n|-----------------------------------------------------|\n");
    
    /* creation de la socket */
    socket_descriptor = create_socket(socket_descriptor);

    /* association du socket socket_descriptor à la structure d'adresse adresse_locale */
    bind_socket(socket_descriptor, adresse_locale);
    
    /* attente des connexions et traitement des donnees recues */

    /*-----------------------------------------------------------
    DEBUT DU JEU
    -----------------------------------------------------------*/

    for(;;) {
       
        /* initialisation de la file d'ecoute */
        listen(socket_descriptor,5);
        
        longueur_adresse_courante = sizeof(adresse_client_courant);
        
        /* adresse_client_courant sera renseigné par accept via les infos du connect */
        nouv_socket_descriptor = accept_client(socket_descriptor, adresse_client_courant, longueur_adresse_courante);

        /* Creation des threads des joueurs si celui du maitre du jeu a reussi. */
        if (! thread_Maitre_Jeu)
        {

            thread_Liaison_Joueur = pthread_create (
            & serveur.thread_joueurs [i], NULL,
            joueur_main, (void*)(intptr_t) nouv_socket_descriptor
            );
            i++;

            if (thread_Liaison_Joueur)
            {
                fprintf (stderr, "%s", strerror (thread_Liaison_Joueur));
            }
        }
    } 
    
//----------------------------------------------------------------------------------------------------------

    /* Attente de la fin des threads. */
    i = 0;
    for (i = 0; i < serveur.tabJoueurs->used; i++)
    {
        pthread_join (serveur.thread_joueurs [i], NULL);
    }
    pthread_join (serveur.thread_serveur, NULL);

    return EXIT_SUCCESS;
}