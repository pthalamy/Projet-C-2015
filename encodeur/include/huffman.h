#ifndef __HUFFMAN_H__
#define __HUFFMAN_H__

#include "bitstream.h"
#include "pack.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

struct abr {
   uint8_t symbole ;
   struct abr *gauche ;
   struct abr *droit ;
   bool est_feuille ;
};


struct elt {
   struct abr *abr ;
   uint8_t occ ;

};

/* Parcours d'un bloc, stockage des symboles dans un tableau*/
void init_freq(int32_t bloc[64],
	       struct elt *freq_DC[256], uint8_t *ind_DC,
	       struct elt *freq_AC[256], uint8_t *ind_AC,
	       int32_t  *pred_DC);



extern struct elt *tab_to_heap(struct elt *tab[256], uint8_t *nb_elt);

void huffman_value(struct abr *abr, uint8_t symbole, uint8_t *code, uint8_t *nb_bits);


extern struct abr *create_huffman_table(struct elt *tab[256], uint8_t *n);

//extern void store_huffman_table(struct bitstream *stream, struct huff_table *ht);

//extern void free_huffman_table(struct huff_table *table);

#endif
