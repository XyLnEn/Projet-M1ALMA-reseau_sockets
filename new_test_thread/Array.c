#include "Array.h"

/* initialisation d'un tableau dynamique de joueurs */
void initArray(Array *a, size_t initialSize) {
  a->array = (Info_player *)malloc(initialSize * sizeof(Info_player));
  a->used = 0;
  a->size = initialSize;
}

/* ajout d'un joueurs dans le tableau */
void insertArray(Array *a, Info_player element) {
  if (a->used == a->size) {
    a->size *= 2;
    a->array = (Info_player *)realloc(a->array, a->size * sizeof(Info_player));
  }
  a->array[a->used++] = element;
}

/* vide le tableau de joueurs */
void freeArray(Array *a) {
  free(a->array);
  a->array = NULL;
  a->used = a->size = 0;
}