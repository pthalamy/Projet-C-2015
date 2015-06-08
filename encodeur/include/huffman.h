#ifndef __HUFFMAN_H__
#define __HUFFMAN_H__

#include "bitstream.h"

#include <stdint.h>
#include <stdbool.h>


struct elt {
   uint8_t symbole ;
   uint8_t occ ;

};

/* Parcours d'un bloc, stockage des symboles dans un tableau*/
void init_freq(int32_t bloc[64],
	       struct elt freq_DC[256], uint8_t ind_DC,
	       struct elt freq_AC[256], uint8_t ind_AC,
	       int32_t  *pred_DC);



struct huff_table;

extern struct huff_table create_huffman_table(struct elt tab[256]);

extern void store_huffman_table(struct bitstream *stream, struct huff_table *ht);

extern void free_huffman_table(struct huff_table *table);

#endif
