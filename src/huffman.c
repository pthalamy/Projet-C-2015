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



/* void construire_arbre (struct abr *huff_tree, */
/* 		       uint8_t *sym, */
/* 		       uint8_t *symOfLen, */
/* 		       uint8_t *symCour, */
/* 		       uint8_t nb_symb) */
/* { */
/*    if (*symCour < nb_symb) { */
/*       huff_tree = malloc (sizeof(struct abr)); */
/*       /\* huff_tree->gauche = malloc (sizeof(struct abr)); *\/ */
/*       /\* huff_tree->droite = malloc (sizeof(struct abr)); *\/ */

/*       if () { */

/*       } */
/*    } */
/* } */

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
   uint32_t buf ;
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



   // création tableau de 256 *abr, rempli de null
   struct abr *tab [256] ;
   for (uint32_t i=0; i<256; i++) {
   tab[i]= NULL;
}

   // on cherche la longueur max des symboles a coder
   uint8_t i = 15;
   while (symbOfLen[i]==0){
   i--;
}

   uint32_t lg_max =i;
   uint32_t nb_lg_max = symbOfLen[i];

   uint32_t nb_symb_codes=0;


   // parcours du tableau de symboles par la fin.
   // création de nouveaux noeuds
   for (uint32_t j=0; j<lg_max; j++) {
   tab[j]=malloc(sizeof(struct abr));
   tab[j]->sym=symboles[nb_symb-nb_lg_max+j];
   tab[j]->gauche = NULL;
   tab[j]->droite= NULL;
}

   nb_symb_codes=nb_symb_codes+nb_lg_max ;


// tant que tout n'a pas été codé
   while (nb_symb_codes!=nb_symb) {

      // regroupement des noeuds par deux pour former l'étage au dessus
      uint8_t k =0;
      for(uint8_t j=0;j<nb_symb_codes; j=j+2){
	 struct abr *temp=malloc (sizeof(struct abr));
	 if (temp==NULL); exit(1);
	 temp->gauche=tab[j];
	 temp->droite=tab[j+1];
	 tab[k]=temp;
	 k++;
      };

      // ajout des nouvelles feuilles
      lg_max--;
      nb_lg_max=symbOfLen[lg_max];

      for (uint8_t j=0; j< nb_lg_max; j++) {
	 struct abr *temp=malloc(sizeof (struct abr));
	 if (temp==NULL); exit(1);
	 temp -> gauche=NULL;
	 temp -> droite = NULL;
	 temp->sym = symboles[nb_symb-nb_symb_codes+j];
	 tab[k]=temp ;
	 k++;
      }


  }
   return tab[1]; // attention il faur retourner une huff table et non pas un arbre
}


int8_t next_huffman_value(struct huff_table *table,
			  struct bitstream *stream)
{
   exit(1);
}

void free_huffman_table(struct huff_table *table)
{

}
