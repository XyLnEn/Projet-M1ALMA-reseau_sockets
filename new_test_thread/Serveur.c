/*----------------------------------------------
Serveur à lancer avant le client avec la commande : ./Serveur.exe
------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <linux/types.h>    /* pour les sockets */
#include <sys/socket.h>
#include <netdb.h> 	        /* pour hostent, servent */
#include <string.h> 
#include <pthread.h>		/* pour bcopy, ... */
#include "Array.h"
#define TAILLE_MAX_NOM 256
#define NB_CLIENTS_MAX 10
#define ATTENTE_DEBUT_PARTIE 10
#define ATTENTE_FIN_JEU 10

#define TAILLE_PHRASE_SANS_CODE 195
#define TAILLE_CODE 5
#define TAILLE_PHRASE_AVEC_CODE 200
#define SCORE_FIN_PARTIE 3


typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;
typedef struct hostent hostent;
typedef struct servent servent;

/* Structure representant une reponse d'un client. */
typedef struct 
{
    int ident;
    char * phrase;
} 
reponse_client;

/* Structure stockant les reponses des clients. */
typedef struct 
{
    reponse_client liste_rep[NB_CLIENTS_MAX];
    int nb_contenu;
}
liste_reponse_clients;


/* Structure stockant les informations des threads clients et du serveur. */
typedef struct
{
   Array * tabClients;
   liste_reponse_clients * tabReponses;
 
   pthread_t thread_serveur;
   pthread_t thread_clients [NB_CLIENTS_MAX];
   pthread_mutex_t mutex_stock;

   int fin_partie;
}
serveur_t;

 //variable globale
serveur_t serveur;


/////////////////////////////////////////////////////////////////////////////////////
/* creation de la socket */
int create_socket(int socket_descriptor) {
    if ((socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("erreur : impossible de creer la socket de connexion avec le client.");
        exit(1);
    }
}

/////////////////////////////////////////////////////////////////////////////////////
/* association du socket socket_descriptor à la structure d'adresse adresse_locale */
void bind_socket(int socket_descriptor, sockaddr_in adresse_locale) {
    if ((bind(socket_descriptor, (sockaddr*)(&adresse_locale), sizeof(adresse_locale))) < 0) {
        perror("erreur : impossible de lier la socket a l'adresse de connexion.");
        exit(1);
    }
}

/////////////////////////////////////////////////////////////////////////////////////
/* réception d'un message envoyé par le serveur */
char * reception(int sock) {
    //char *buffer = malloc(256*sizeof(char));//laisser en tableau sinon envoi 8 lettres par 8
    char * buffer = malloc(TAILLE_PHRASE_AVEC_CODE * sizeof(char));
    char * cleaned_sentence = malloc(TAILLE_PHRASE_AVEC_CODE * sizeof(char));
    strcpy(cleaned_sentence,"");
    // strcpy()
    int longueur;
   
    if ((longueur = read(sock, buffer, TAILLE_PHRASE_AVEC_CODE)) <= 0){ 
        return "";
    }

    
    //traitement du message 
    int i;
    for(i = 0; i < longueur; i++) {
        if(buffer[i] == '\n') {
            buffer[i] = '\0';
        }
    }


    memcpy(cleaned_sentence, buffer, longueur);
    cleaned_sentence[longueur+1] ='\0';

    // buffer[strlen(buffer)-1] ='\0';//attention erreur potentielle

    //printf("marche1?");
    printf("reception d'un message de taille %d : %s|\n", longueur, cleaned_sentence);

    //printf("marche2?");
    return cleaned_sentence;
}

////////////////////////////////////////////////////////////////////////////////////
/*
void renvoi (int sock) {

    char buffer[TAILLE_PHRASE_AVEC_CODE];
    int longueur;
   
    if ((longueur = read(sock, buffer, sizeof(buffer))) <= 0) 
    	return;
    //
	 //traitement du message 
	printf("reception d'un message.\n");
    printf("message lu : %s \n", buffer);
    
    buffer[longueur+1] ='\0';
    return;
}

*/
////////////////////////////////////////////////////////////////////////////////////
//genere la concat de code~phrase proprement
char * crea_phrase(char * mot, char * code) {

    char * fin = malloc((strlen(mot) + strlen(code)+1) * sizeof(char));
    memcpy(fin, code, strlen(code));
    memcpy(fin + strlen(code), "~", 1);
    memcpy(fin + strlen(code) + 1, mot, strlen(mot));
    return fin;
}

/////////////////////////////////////////////////////////////////////////////////////
void prevenir_leader() {
    char * trigger = malloc(7*sizeof(char));//on connait exactement la taille
    trigger = crea_phrase("go","0003");
    int i = 0;
    for (i = 0; i < serveur.tabClients->used; ++i) {
            // printf("maybe? %d\n",serveur.tabClients->array[i].leader);
       if(serveur.tabClients->array[i].leader == 1) {
            write(serveur.tabClients->array[i].socket,trigger,strlen(trigger));

       } 
    }

}

/////////////////////////////////////////////////////////////////////////////////////
void choix_leader() {
    if(serveur.tabClients->used == 0) {
        perror("erreur : choix leader impossible");
        exit(1);
    } else {
        if(serveur.tabClients->used == 1) {
            serveur.tabClients->array[0].leader = 1;
        } else if(serveur.tabClients->array[serveur.tabClients->used-1].leader == 1) {
            serveur.tabClients->array[serveur.tabClients->used-1].leader = 0;
            serveur.tabClients->array[0].leader = 1;
        }else {
            int i = 0;
            int j = 0;
            for (i = 0; i < serveur.tabClients->used - 1; ++i) {
                if(serveur.tabClients->array[i].leader == 1) {
                    j = 1;
                    serveur.tabClients->array[i].leader = 0;
                    serveur.tabClients->array[i+1].leader = 1;
                    i = serveur.tabClients->used;
                } 
            }
            if(j == 0) { //pas trouvé de leader
                serveur.tabClients->array[0].leader = 1;
            }
        }
        affich_clients();
        prevenir_leader();
    }
    return;
}

/////////////////////////////////////////////////////////////////////////////////////
/* conversion code to int */
int convert_code(char * s) {
    return (((s[0] - '0')*1000) + ((s[1] - '0')*100) + ((s[2] - '0')*10) + ((s[3] - '0')));
}

/////////////////////////////////////////////////////////////////////////////////
/*previens les client du gagnant d'un tour */
void prevenir_clients(int k, char * suiv) {
    char * reponse = malloc(TAILLE_PHRASE_AVEC_CODE * sizeof(char));
    int w;
    memcpy(reponse, serveur.tabClients->array[k].pseudo, strlen(serveur.tabClients->array[k].pseudo));
    // memcpy(reponse + strlen(serveur.tabClients->array[k].pseudo) , " a gagne ce tour!", 9);
    memcpy(reponse + strlen(serveur.tabClients->array[k].pseudo) , suiv, strlen(suiv));
    reponse = crea_phrase(reponse,"0002");
    for(w = 0; w < serveur.tabClients->used; w++) {
        write(serveur.tabClients->array[w].socket,reponse,strlen(reponse));
        sleep(1);
    }
}

/////////////////////////////////////////////////////////////////////////////////////
// char * decode(char * test, int nouv_socket_descriptor, Array * tabClients) {
void decode(char * test, int nouv_socket_descriptor, Array * tabClients) {

    if (strstr(test,"~")) {
        Info_player element;//exemple de creation d'un element -> a faire dans le thread
        int i;
        int j;
        int k;

        char * code;
        char * phrase;

        code = malloc(TAILLE_CODE*sizeof(char));
        strcpy(code,"");
        code = strtok(test,"~");
        // printf("%s|\n",code);
        phrase = malloc(TAILLE_PHRASE_SANS_CODE*sizeof(char));
        strcpy(phrase,"");
        phrase = strtok(NULL,"~");
        // printf("%s|\n",phrase);
        char * reponse;
        reponse = malloc(TAILLE_PHRASE_AVEC_CODE * sizeof(char));
        strcpy(reponse,"");
        i = convert_code(code);
        //on regarde ici le code et on reagit en consequence
        if (i == 0){
            element.socket = nouv_socket_descriptor;//a faire apres la connexion!
            
            element.pseudo = malloc(TAILLE_PHRASE_SANS_CODE*sizeof(char));
            strcpy(element.pseudo,phrase);

            element.score = 0;
            element.leader = 0;
            insertArray(serveur.tabClients, element);
            // printf("ici used = %zu \n", serveur.tabClients->used);//pour voir que chaque nouvelle connexion de client est vue
            // for(i = 0; i < serveur.tabClients->used; i++) {
            //     printf("leader: %s : %d \n", serveur.tabClients->array[i].pseudo, serveur.tabClients->array[i].socket);//affichage de tout les joueurs avec le socket sur lequel les contacter.
            // }
            //reponse = crea_phrase(phrase,"0003");//a changer pour envoyer autre type de messages

            return;
        }
        else if (i == 1) {

            reponse = crea_phrase(phrase,"0001");//a changer pour envoyer autre type de messages
            printf("****************************************************************%s*\n",reponse);
            for (j = 0; j < serveur.tabClients->used; ++j) {
                if(serveur.tabClients->array[j].leader == 0) {


                    write(serveur.tabClients->array[j].socket,reponse,strlen(reponse));

                } 
            }
        }
        else if (i == 2) {
            pthread_mutex_lock (& serveur.mutex_stock);
            
            if(serveur.fin_partie == 0) {

                printf("premiere reponse\n");
                serveur.fin_partie = 1;
            }

            //ecriture du couple phrase/no dans le tab de reponses...
            reponse_client * rep = malloc(sizeof(reponse_client));
            rep->ident = nouv_socket_descriptor;

            rep->phrase = malloc(strlen(phrase) * sizeof(char) );
            strcpy(rep->phrase,phrase);
            // rep->phrase = phrase;

            serveur.tabReponses->liste_rep[serveur.tabReponses->nb_contenu] = *rep;
            serveur.tabReponses->nb_contenu++;

            // printf("on a ecrit dans le tab de reponses: %s\n",phrase);
            // printf("etat actuel: %d rep:\n",serveur.tabReponses->nb_contenu);
            // for(j = 0; j < serveur.tabReponses->nb_contenu; j++) {
            //     printf("%d :",j);
            //     printf("%s\n",serveur.tabReponses->liste_rep[j].phrase);
            // }
            printf("******************\n");


            //liberation du mutex
            pthread_mutex_unlock (& serveur.mutex_stock);
        }
        else if (i == 4) {
            j = phrase[0] - '0';
            for(k = 0; k < serveur.tabClients->used; k++) {
                if(serveur.tabClients->array[k].socket == j) { //trouvé le gagnant!
                    serveur.tabClients->array[k].score++;
                    if(serveur.tabClients->array[k].score >= SCORE_FIN_PARTIE) {
                        strcpy(reponse, " a gagne la partie, CONGRATULATION!");
                        printf("%s est le gagnant! CONGRATULATION!\n",serveur.tabClients->array[k].pseudo);
                        prevenir_clients(k,reponse);
                    } else {
                        strcpy(reponse, " a gagne ce tour!");
                        printf("%s remporte le point!\n",serveur.tabClients->array[k].pseudo);
                        serveur.tabReponses->nb_contenu = 0;
                        prevenir_clients(k,reponse);
                        choix_leader();
                    }
                
                }
            }
        }


    }
    return;
}

/////////////////////////////////////////////////////////////////////////////////////
/* connexion d'un nouveau client */
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
void affich_clients() {
    printf("pseudo\t score\t leader\n");
    int i = 0;
    for (i = 0; i < serveur.tabClients->used; ++i) {
        printf("%s\t %d\t %d\t %d\n", 
             serveur.tabClients->array[i].pseudo, serveur.tabClients->array[i].score,
             serveur.tabClients->array[i].leader, serveur.tabClients->array[i].socket);
    }
    return;
}

/////////////////////////////////////////////////////////////////////////////////////
void envoi_resultat_leader() {

    char * reponse; 
    char * phrase;
    char nb[5];
    int i;
    int j;
    for (i = 0; i < serveur.tabClients->used; ++i) {
        if(serveur.tabClients->array[i].leader == 1) {
            j = serveur.tabClients->array[i].socket;
            i = serveur.tabClients->used;
        } 
    }
    for (i = 0; i < serveur.tabReponses->nb_contenu; ++i)
    {
        reponse = malloc((TAILLE_PHRASE_AVEC_CODE + 3)*sizeof(char)); //+3 char pour nb|
        phrase = malloc (TAILLE_PHRASE_SANS_CODE * sizeof(char));

        sprintf(nb, "%d", serveur.tabReponses->liste_rep[i].ident);
        memcpy(phrase, nb, strlen(nb));
        memcpy(phrase + strlen(nb), "|", 1);
        memcpy(phrase + strlen(nb) + 1, serveur.tabReponses->liste_rep[i].phrase , strlen(serveur.tabReponses->liste_rep[i].phrase));
        
        reponse = crea_phrase(phrase,"0002");

        printf("%d, %s de taille %zd\n",i,reponse, strlen(reponse));
        
        write(j, reponse, strlen(reponse));
        sleep(1);
    }
    write(j, "0003~go", 7*sizeof(char));

    serveur.fin_partie = 0;
    return;
}

/////////////////////////////////////////////////////////////////////////////////////
/* Fonction pour le thread du magasin. */
static void * mj_main (void * p_data) {

    initArray(serveur.tabClients, 5);
    sleep (ATTENTE_DEBUT_PARTIE);

    while(serveur.tabClients->used < 3) {
        sleep(1);
    }
    choix_leader();
    affich_clients();
    while (1) {
         /* Debut de la zone protegee. */
      pthread_mutex_lock (& serveur.mutex_stock);
      if(serveur.fin_partie == 1) {//on a reçu au moins 1 reponse, debut du compte a rebour
        pthread_mutex_unlock (& serveur.mutex_stock);

        //printf("azyyyyyyyyyyyyyyyyyLOLOLOLOLO");

        sleep(ATTENTE_FIN_JEU);//attente de 40 sec

        pthread_mutex_lock (& serveur.mutex_stock);

        printf("fin du jeu, choix du gagnant: \n");

        envoi_resultat_leader();

      }
      pthread_mutex_unlock (& serveur.mutex_stock);
    }
    return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////
/* Fonction pour les threads des clients. */
static void * joueur_main (void * p_data)
{
    char * pseudo;
    char * temp;
    int nouv_socket_descriptor = (int) p_data;
 
   while (1)
   {

        pseudo = reception(nouv_socket_descriptor);
        decode(pseudo,nouv_socket_descriptor, serveur.tabClients);
        // sleep(1);//?????????

   }
 
   return NULL;
}
/////////////////////////////////////////////////////////////////////////////////////
int main (void)
{
	serveur.tabClients = malloc(sizeof(Array));
    serveur.tabReponses = malloc(sizeof(liste_reponse_clients));
    serveur.mutex_stock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;

    int socket_descriptor, 		    /* descripteur de socket */
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
    adresse_locale.sin_family		= ptr_hote->h_addrtype; 	/* ou AF_INET */
    adresse_locale.sin_addr.s_addr	= INADDR_ANY; 			/* ou AF_INET */

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
    
    printf("\n-----------------------------------------\n");
    printf("\n-->Numero de port pour la connexion au serveur : %d \n", 
           ntohs(adresse_locale.sin_port) /*ntohs(ptr_service->s_port)*/);
    
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

        //         if ((nouv_socket_descriptor = 
        //     accept(socket_descriptor, 
        //            (sockaddr*)(&adresse_client_courant),
        //            &longueur_adresse_courante))
        //      < 0) {
        //     perror("erreur : impossible d'accepter la connexion avec le client.");
        //     exit(1);
        // }

		/* Creation des threads des joueurs si celui du maitre du jeu a reussi. */
		if (! thread_Maitre_Jeu)
		{

			thread_Liaison_Joueur = pthread_create (
			& serveur.thread_clients [i], NULL,
			joueur_main, (void *) nouv_socket_descriptor
		 	);
            i++;

			if (thread_Liaison_Joueur)
			{
				fprintf (stderr, "%s", strerror (thread_Liaison_Joueur));
			}
		}

    
        
		// else
		// {
		// 	fprintf (stderr, "%s", strerror (ret));
		// }

		//close(nouv_socket_descriptor);//garder?
    } 
    
//----------------------------------------------------------------------------------------------------------

	/* Attente de la fin des threads. */
	i = 0;
	for (i = 0; i < serveur.tabClients->used; i++)
	{
		pthread_join (serveur.thread_clients [i], NULL);
	}
	pthread_join (serveur.thread_serveur, NULL);

	return EXIT_SUCCESS;
}