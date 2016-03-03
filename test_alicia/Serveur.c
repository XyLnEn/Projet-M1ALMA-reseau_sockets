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
#define ATTENTE_DEBUT_PARTIE 20

typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;
typedef struct hostent hostent;
typedef struct servent servent;

/* Structure stockant les informations des threads clients et du serveur. */
typedef struct
{
   Array * tabClients;
 
   pthread_t thread_serveur;
   pthread_t thread_clients [NB_CLIENTS_MAX];
   //pthread_mutex_t mutex_stock;
}
serveur_t;
 
 //variable globale
serveur_t serveur;
// static serveur_t serveur =
// {
//     serveur.tabClients = malloc(sizeof(Array));
// };

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
    //char *buffer = malloc(256*sizeof(char));
    static char buffer[256];
    int longueur;
   
    if ((longueur = read(sock, buffer, sizeof(buffer))) <= 0){ 
        return "";
    }
    
    //traitement du message 
    int i;
    for(i = 0; i < longueur; i++) {
        if(buffer[i] == '\n') {
            buffer[i] = '\0';
        }
    }
    // buffer[strlen(buffer)-1] ='\0';//attention erreur potentielle

    printf("reception d'un message de taille %d : %s\n", longueur, buffer);
    return buffer;
}

/////////////////////////////////////////////////////////////////////////////////////
/* réception d'un message envoyé par le serveur */
char * recep_pseudo(int sock) {
    //char *buffer = malloc(256*sizeof(char));
    static char buffer[50];
    int longueur;
   
    if ((longueur = read(sock, buffer, sizeof(buffer))) <= 0){ 
        return "";
    }
    
    //traitement du message 
    int i;
    for(i = 0; i < longueur; i++) {
        if(buffer[i] == '\n') {
            buffer[i] = '\0';
        }
    }

    return buffer;
}

/////////////////////////////////////////////////////////////////////////////////////
void envoi (int sock) {

    char buffer[256];
    int longueur;
   
    if ((longueur = read(sock, buffer, sizeof(buffer))) <= 0) 
    	return;
    
	 //traitement du message 
	printf("reception d'un message.\n");
    printf("message lu : %s \n", buffer);
    
    buffer[longueur+1] ='\0';
    return;
}

/////////////////////////////////////////////////////////////////////////////////////
void decode(char * test, int nouv_socket_descriptor, Array * tabClients) {

    if (strstr(test,"~")) {
        Info_player element;//exemple de creation d'un element -> a faire dans le thread
        int i;

        char * code;
        char * phrase;
        code = malloc(5*sizeof(char));
        code = strtok(test,"~");
        //printf("%s\n",code);
        phrase = malloc(256*sizeof(char));
        phrase = strtok(NULL,"~");
        //printf("%s\n",phrase);

        if ((code[3] == '0')){
            // element = (Info_player)malloc(sizeof(Info_player));
            element.socket = nouv_socket_descriptor;//a faire apres la connexion!
            element.pseudo = phrase;
            element.score = 0;
            element.leader = 0;
            insertArray(tabClients, element);
            // printf("ici used = %zu \n", tabClients->used);//pour voir que chaque nouvelle connexion de client est vue
            // for(i = 0; i < tabClients->used; i++) {
            //     printf("leader: %s : %d \n", tabClients->array[i].pseudo, tabClients->array[i].socket);//affichage de tout les joueurs avec le socket sur lequel les contacter.
            // }
        }
    }
    return;
}

/////////////////////////////////////////////////////////////////////////////////////
/* connexion d'un nouveau client */
int accept_client(int socket_descriptor, sockaddr_in adresse_client_courant, 
    int longueur_adresse_courante, Array * tabClients, char * pseudo) {

    int nouv_socket_descriptor;
    if ((nouv_socket_descriptor = accept(socket_descriptor, 
      (sockaddr*)(&adresse_client_courant), &longueur_adresse_courante)) < 0) {
        perror("erreur : impossible d'accepter la connexion avec le client.");
        exit(1);
    } else {
        pseudo = reception(socket_descriptor);
        decode(pseudo,socket_descriptor, tabClients);
    }
    return nouv_socket_descriptor;
}

/////////////////////////////////////////////////////////////////////////////////////
/* Fonction pour le thread du magasin. */
// static void * fn_serveur (void * p_data) {

//     //initArray(serveur.tabClients, 5);
//    // sleep (ATTENTE_DEBUT_PARTIE);

//     while (1) {
//          /* Debut de la zone protegee. */
//       //pthread_mutex_lock (& serveur.mutex_stock);
 
//       //choix_leader(serveur.tabClients);
 
//       /* Fin de la zone protegee. */
//       //pthread_mutex_unlock (& serveur.mutex_stock);
//     }
//     return NULL;
// }

/////////////////////////////////////////////////////////////////////////////////////
/* Fonction pour les threads des clients. */
static void * fn_clients (void * p_data)
{
	char * pseudo;
	int nouv_socket_descriptor = (int) p_data;
 
   while (1)
   {

        pseudo = reception(nouv_socket_descriptor);
        decode(pseudo,nouv_socket_descriptor, serveur.tabClients);
   }
 
   return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////
void choix_leader(Array * tabClients) {
    if(tabClients->used == 0) {
        perror("erreur : choix leader impossible");
        exit(1);
    } else if(tabClients->used == 1) {
        tabClients->array[0].leader = 1;
    } else if(tabClients->array[tabClients->used].leader == 1) {
        tabClients->array[tabClients->used].leader == 0;
        tabClients->array[0].leader == 1;
    }else {
        int i = 0;
        for (i = 0; i < tabClients->used - 1; ++i) {
           if(tabClients->array[i].leader == 1) {
                tabClients->array[i].leader = 0;
                tabClients->array[i+1].leader = 1;
                return;
           } 
        }
    }
    return;
}

/////////////////////////////////////////////////////////////////////////////////////
void affich_clients(Array * tabClients) {
    printf("pseudo --- score --- leader\n");
    int i = 0;
    for (i = 0; i < tabClients->used; ++i) {
        printf("%s --- %d --- %d --- %d\n", 
             tabClients->array[i].pseudo, tabClients->array[i].score,
             tabClients->array[i].leader, tabClients->array[i].socket);
    }
    return;
}

/////////////////////////////////////////////////////////////////////////////////////
int main (void)
{
	 serveur.tabClients = malloc(sizeof(Array));
 //    serveur.mutex_stock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;;

    int socket_descriptor, 		    /* descripteur de socket */
		nouv_socket_descriptor,     /* [nouveau] descripteur de socket */
		longueur_adresse_courante;  /* longueur d'adresse courante d'un client */
    sockaddr_in adresse_locale,     /* structure d'adresse locale*/
		adresse_client_courant;     /* adresse client courant */
    hostent * ptr_hote;             /* les infos recuperees sur la machine hote */
    servent * ptr_service;          /* les infos recuperees sur le service de la machine */
    char machine[TAILLE_MAX_NOM+1]; /* nom de la machine locale */

    //Array * tabClients = malloc(sizeof(Array));
    initArray(serveur.tabClients, 5);

    char * pseudo = "";

	int i = 0;
    int ret = 0;
    int thread_pere = 0;
    int thread_fils = 0;
 
	/* Creation du thread du serveur. */
	// printf ("Creation du thread du serveur !\n");
	// thread_pere = pthread_create (
	// 	& serveur.thread_serveur, NULL, fn_serveur, NULL);


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
    printf("-->Serveur lancé.\n");
    printf("\n-----------------------------------------\n");

    /*-----------------------------------------------------------
    DEBUT DU JEU
    -----------------------------------------------------------*/

    for(;;) {
	   
    	/* initialisation de la file d'ecoute */
    	listen(socket_descriptor,5);
    	
		longueur_adresse_courante = sizeof(adresse_client_courant);
		
		/* adresse_client_courant sera renseigné par accept via les infos du connect */
		nouv_socket_descriptor = accept_client(socket_descriptor, adresse_client_courant,
         longueur_adresse_courante, serveur.tabClients, pseudo);

        //         if ((nouv_socket_descriptor = 
        //     accept(socket_descriptor, 
        //            (sockaddr*)(&adresse_client_courant),
        //            &longueur_adresse_courante))
        //      < 0) {
        //     perror("erreur : impossible d'accepter la connexion avec le client.");
        //     exit(1);
        // }

        //pseudo = reception(nouv_socket_descriptor);
        //decode(pseudo,nouv_socket_descriptor, serveur.tabClients);
        
        choix_leader(serveur.tabClients);
        affich_clients(serveur.tabClients);
		/* Creation des threads des clients si celui du magasin a reussi. */
		if (! thread_pere)
		{
			thread_fils = pthread_create (
			& serveur.thread_clients [i], NULL,
			fn_clients, (void *) nouv_socket_descriptor
		 	);

			if (thread_fils)
			{
				fprintf (stderr, "%s", strerror (thread_fils));
			}
		}
		else
		{
			fprintf (stderr, "%s", strerror (ret));
		}

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