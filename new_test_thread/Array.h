#ifndef ARRAY_HPP
#define ARRAY_HPP

#include <stddef.h>
#include <stdlib.h>

/* structure de clients */
typedef struct {
	int socket;
   char * pseudo;
   int score;
   int leader;
   pthread_t assigned_thread;
} Info_player;

/* structure de tableau dynamique */
typedef struct {
  Info_player * array;
  size_t used;
  size_t size;
} Array;

/* initialisation d'un tableau dynamique de clients */
void initArray(Array *a, size_t initialSize);
/* ajout d'un client dans le tableau */
void insertArray(Array *a, Info_player element);
/* vide le tableau de clients */
void freeArray(Array *a);

#endif //ARRAY_HPP