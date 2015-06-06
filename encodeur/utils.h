#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdlib.h>
#include <stdio.h>

/* Retourne un pointeur sur une zone de mémoire allouée non initialisée de taille size,
 * et leve une erreur d'execution si l'allocation echoue*/
void *smalloc(size_t size);

#endif
