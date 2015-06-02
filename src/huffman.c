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
   bool est_feuille;
   struct abr *gauche, *droite;
};

struct huff_table {
   struct abr *huff_tree;
};


void affiche_huffman_rec (struct abr *huff, uint8_t code[16], uint8_t nbits)
{
   /* Huff est une feuille, affichage du code et de la valeur du noeud */
   if (huff->est_feuille) {
      for (uint8_t i = 0; i < nbits; i++)
	 printf ("%d", code[i]);

      printf (" -> 0x%x\n", huff->sym);
   } else {
      /* Visite fils gauche */
      if (huff->gauche) {
	 code[nbits] = 0;
	 affiche_huffman_rec (huff->gauche, code, nbits+1);
      }

      /* Visite fils droit */
      if (huff->droite) {
	 code[nbits] = 1;
	 affiche_huffman_rec (huff->droite, code, nbits+1);
      }
   }
}

void affiche_huffman (struct abr *huff)
{
   if (huff) {
      uint8_t code[16];
      uint8_t nbits = 0;
      affiche_huffman_rec (huff, code, nbits);
   }
}

bool insertion(struct abr **abr, uint8_t depth, uint8_t sym)
{
   /* On descend jusqu'à la profondeur à atteindre */
   /* printf ("depth: %d\t", depth); */
   /* printf ("abr: %p\n", (*abr)); */
   if (depth) {

      if (!(*abr)) {
	 /* printf ("Création non-feuille: \n"); */
	 /* Création d'un noeud intermédiaire */
	 (*abr) = malloc (sizeof(struct abr));
	 (*abr)->est_feuille = false;
	 (*abr)->gauche = NULL;
	 (*abr)->droite = NULL;
      }

      if (!(*abr)->est_feuille) {
	 /* Tentative d'insertion dans le sous-arbre, avec priorité à gauche */
	 if (insertion (&((*abr)->gauche), depth - 1, sym)) return true;
	 else return insertion (&((*abr)->droite), depth - 1, sym);
      }

   } else {

      /* Si le noeud est inexistant, on créée la feuille ici, sinon on tente un autre sous-arbre*/
      if (!(*abr)) {
	 /* printf ("Création feuille: 0x%x \t %p\n", sym, abr); */
	 (*abr) = malloc (sizeof(struct abr));
	 (*abr)->est_feuille = true;
	 (*abr)->sym = sym;
	 (*abr)->gauche = NULL;
	 (*abr)->droite = NULL;
	 return true;
      }

   }
   return false;
}

struct huff_table *load_huffman_table(
   struct bitstream *stream, uint16_t *nb_byte_read)
{
   *nb_byte_read = 0;

   struct huff_table *ht = malloc (sizeof(struct huff_table));
   if (!ht) {
      *nb_byte_read = -1;
      return NULL;
   }

   /* Recuperation du nombre de symboles de chaque longueur */
   uint16_t nb_symb = 0;
   uint8_t symbOfLen[16];
   uint32_t buf ;
   /* printf ("Symboles de longueur: \n"); */
   for (uint8_t i = 0; i < 16; i++) {
      read_bitstream (stream, 8, &buf, false);
      (*nb_byte_read)++;
      symbOfLen[i] = buf;
      nb_symb += symbOfLen[i];
      /* printf ("%d ", symbOfLen[i]); */
   }

   /* printf ("\nNombre de symboles: %d\n", nb_symb); */
   assert (nb_symb < 256);
   uint8_t symboles[nb_symb];

   /* printf ("{ "); */
   for (uint8_t j = 0; j < nb_symb; j++) {
      read_bitstream (stream, 8, &buf, true);
      (*nb_byte_read)++;
      symboles[j] = buf;
      /* printf ("0x%x ", buf); */
   }
   /* printf ("};\n"); */

   /* printf ("nb_byte_read: %d\n", *nb_byte_read); */

   /* printf ("\n"); */

// Construction de l'arbre
   ht->huff_tree = NULL;
   uint8_t curSym = 0;		/* Indice du symbole courant */
   /* Pour chaque longueur de code, insertion des symboles correspondant dans l'arbre */
   for (uint32_t i = 1; i <= 16; i++) {
      for (uint32_t j = 0; j < symbOfLen[i-1]; j++) {
	 /* printf ("Insertion: depth = %d | symbole = 0x%x\n", i, symboles[curSym]); */
	 insertion (&ht->huff_tree, i, symboles[curSym++]);
      }
   }

   return ht;
}

uint8_t count = 0;
void rec_parcours_abr(struct abr *arbre, uint32_t direction, uint8_t *symbole, struct bitstream *stream){
   /* printf ("count: %d\n", ++count); */
   if (arbre == NULL) {
      /* printf("arbre null\n"); */
      return ;
   } else if (arbre->est_feuille) {
      *symbole = arbre -> sym;
      /* printf("feuille atteinte : 0x%x \n", *symbole); */
      return ;
   } else {
      read_bitstream(stream, 1, &direction, true);
      /* printf ("%d", direction); */

      if (direction == 1){
	 rec_parcours_abr(arbre->droite,direction , symbole, stream);
	 /* printf("dir : %i, symbole : %i/ parcours droit \n", direction, *symbole ); */
      } else if (direction == 0){
	 rec_parcours_abr(arbre->gauche, direction, symbole, stream);
	 /* printf("dir : %i, symbole : %i/ parcours droit \n", direction, *symbole ); */
      } else {
	 printf("erreur dans la lecture bit !=0ou1\n");
	 exit(1);
      }
   }

}



int8_t next_huffman_value(struct huff_table *table,
			  struct bitstream *stream)
{
   uint32_t direction = 2;	/* avoid uninitialized use */
   uint8_t symb = 0;
   rec_parcours_abr(table->huff_tree, direction, &symb,  stream );
   return symb;
}

void libere_huffman (struct abr *huff)
{
   if (huff) {
      /* libere sous-arbre gauche */
      libere_huffman (huff->gauche);

      /* Libere sous-arbre droit */
      libere_huffman (huff->droite);

      free (huff);
      huff = NULL;
   }
}

void free_huffman_table(struct huff_table *table)
{
   if (table) {
      libere_huffman (table->huff_tree);
      free (table);
      table = NULL;
   }
}
