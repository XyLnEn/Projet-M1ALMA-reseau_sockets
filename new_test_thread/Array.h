#ifndef ARRAY_HPP
#define ARRAY_HPP

#include <stddef.h>
#include <stdlib.h>

/* structure de joueurs */
typedef struct {
	int socket;
   char * pseudo;
   int score;
   int leader; /* boolean : 1 pour leader, 0 pour non leader */
   pthread_t assigned_thread;
} Info_player;

/* structure de tableau dynamique */
typedef struct {
  Info_player * array;
  size_t used;
  size_t size;
} Array;

/* initialisation d'un tableau dynamique de joueurs */
void initArray(Array *a, size_t initialSize);

/* ajout d'un joueurs dans le tableau */
void insertArray(Array *a, Info_player element);

/* vide le tableau de joueurs */
void freeArray(Array *a);

#endif //ARRAY_HPP