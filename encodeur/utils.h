#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdlib.h>
#include <stdio.h>

#define BITMASK_R(n) (0xFFFFFFFF >> (32 - (n)))
#define BITMASK_L(n) (0xFFFFFFFF << (32 - (n)))

/* Retourne un pointeur sur une zone de mémoire allouée non initialisée de taille size,
 * et leve une erreur d'execution si l'allocation echoue*/
extern void *smalloc(size_t size);

#endif
