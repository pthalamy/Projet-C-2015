#include "huffman.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#define BIT(x, n) ((x >> n) & 1)
#define BITMASK(n) (0xFFFFFFFF >> (32 - (n)))

enum AC_DC {
   AC,
   DC
};

struct abr {
   uint32_t sym;
   struct abr *gauche, *droite;
};

struct huff_table {
   enum AC_DC type;
   uint8_t ind;
   struct abr *huff_tree;
};

void construire_arbre (struct abr *huff_tree,
		       uint8_t *sym,
		       uint8_t *symOfLen,
		       uint8_t *symCour,
		       uint8_t nb_symb)
{
   if (*symCour < nb_symb) {
      huff_tree = malloc (sizeof(struct abr));
      /* huff_tree->gauche = malloc (sizeof(struct abr)); */
      /* huff_tree->droite = malloc (sizeof(struct abr)); */

      if () {

      }
   }
}

struct huff_table *load_huffman_table(
   struct bitstream *stream, uint16_t *nb_byte_read)
{
   *nb_byte_read = 0;

   struct huff_table *huff = malloc (sizeof(struct huff_table));
   if (!huff) {
      *nb_byte_read = -1;
      return NULL;
   }

   /* Recuperation du nombre de symboles de chaque longueur */
   uint16_t nb_symb = 0;
   uint8_t symbOfLen[16];
   printf ("Symboles de longueur: \n");
   for (uint8_t i = 0; i < 16; i++) {
      read_bitstream (stream, 8, &buf, false);
      (*nb_byte_read)++;
      symbOfLen[i] = 0xff & buf;
      nb_symb += symbOfLen[i];
      printf ("%d ", symbOfLen[i]);
   }

   printf ("\nNombre de symboles: %d\n", nb_symb);
   assert (nb_symb < 256);
   uint8_t symboles[nb_symb];

   printf ("{ ");
   for (uint8_t j = 0; j < nb_symb; j++) {
      read_bitstream (stream, 8, &buf, false);
      (*nb_byte_read)++;
      symboles[j] = buf & 0xff;
      printf ("0x%x ", buf);
   }
   printf ("};\n");

   printf ("nb_byte_read: %d\n", *nb_byte_read);

   printf ("\n");
}

int8_t next_huffman_value(struct huff_table *table,
			  struct bitstream *stream)
{
   exit (1);
}

void free_huffman_table(struct huff_table *table)
{

}
