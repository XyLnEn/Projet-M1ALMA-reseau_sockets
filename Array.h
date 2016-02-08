#ifndef ARRAY_HPP
#define ARRAY_HPP

#include <stddef.h>
#include <stdlib.h>

// using namespace std;

typedef struct {
    char * pseudo;
    int score;
} Info_player;

typedef struct {
  Info_player * array;
  size_t used;
  size_t size;
} Array;

void initArray(Array *a, size_t initialSize);
void insertArray(Array *a, Info_player element);
void freeArray(Array *a);

#endif //ARRAY_HPP