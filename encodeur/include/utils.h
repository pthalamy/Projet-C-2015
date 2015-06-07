#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#define BITMASK_R(n) (0xFFFFFFFF >> (32 - (n)))
#define BITMASK_L(n) (0xFFFFFFFF << (32 - (n)))

/* Retourne un pointeur sur une zone de mémoire allouée non initialisée de taille size,
 * et leve une erreur d'execution si l'allocation echoue*/
extern void *smalloc(size_t size);

extern void *scalloc(size_t num, size_t size);

uint16_t le16_to_cpu(const uint16_t v);
uint32_t le32_to_cpu(const uint32_t v);

uint16_t be16_to_cpu(const uint16_t v);
uint32_t be32_to_cpu(const uint32_t v);

#endif
