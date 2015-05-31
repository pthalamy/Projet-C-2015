#include "huffman.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#define BIT(x, n) ((x >> n) & 1)

const uint16_t putmask[16] = {
   0x01, /* 0b0000 0001 */
   0x03, /* 0b0000 0011 */
   0x07, /* 0b0000 0111 */
   0x0f, /* 0b0000 1111 */
   0x1f, /* 0b0001 1111 */
   0x3f, /* 0b0011 1111 */
   0x7f,  /* 0b0111 1111 */
   0xff,
   0x1ff,
   0x3ff,
   0x7ff,
   0xfff,
   0x1fff,
   0x3fff,
   0x7fff,
   0xffff
};

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
		       uint8_t nb_symb)
{
   huff_tree = malloc (sizeof(struct abr));
   huff_tree->gauche = malloc (sizeof(struct abr));
   huff_tree->droite = malloc (sizeof(struct abr));

   uint8_t cur_level = 1;
   struct abr *cour = huff_tree;

   if (symbOfLen[cur_level] == 0) {
      /* cour->gauche =  */
   }
}

struct huff_table *load_huffman_table(
   struct bitstream *stream, uint16_t *nb_byte_read)
{
   uint32_t buf = 0;

   struct huff_table *huff = malloc (sizeof(struct huff_table));

/* Positionner le flux après la balise DHT + 3 */
   skip_bitstream_until (stream, 0xc4);
   read_bitstream (stream, 8, &buf, false); /* skip 0xc4 */
   buf = 0;

   /* Lecture de la taille de la section */
   uint32_t seclen = 0;
   read_bitstream (stream, 16, &seclen, false);
   printf ("Longueur de la section: %d\n", seclen);

   /* Lecture des 3 bits suivants, doivent valoir 0 */
   read_bitstream (stream, 3, &buf, false);
   assert (!buf);

   /* Lecture du bit de type */
   read_bitstream (stream, 1, &buf, false);
   if (BIT (buf, 0)) {
      printf ("type AC\n");
      huff->type = AC;
   } else {
      printf ("type DC\n");
      huff->type = DC;
   }

   /* Lecture de l'indice */
   read_bitstream (stream, 4, &buf, false);
   huff->ind = 0x0f & buf;
   printf ("indice : %d\n", huff->ind);

   /* Recuperation du nombre de symboles de chaque longueur */
   uint16_t nb_symb = 0;
   uint8_t symbOfLen[16];
   printf ("Symboles de longueur: \n");
   for (uint8_t i = 0; i < 16; i++) {
      read_bitstream (stream, 8, &buf, false);
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
      symboles[j] = buf & 0xff;
      printf ("0x%x ", buf);
   }
   printf ("};\n");

   *nb_byte_read = 2 + seclen;

   printf ("\n");
/* En cas d'echec : */
   /* *nb_byte_read = -1; */
   /* return NULL; */
}

int8_t next_huffman_value(struct huff_table *table,
			  struct bitstream *stream)
{
   exit (1);
}

void free_huffman_table(struct huff_table *table)
{

}
