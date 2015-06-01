
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

struct abr {
   uint32_t sym;
   bool est_feuille;
   struct abr *gauche, *droite;
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

void libere_huffman (struct abr *huff)
{
   if (huff) {
      /* libere sous-arbre gauche */
      if (huff->gauche) {
	 libere_huffman (huff->gauche);
      }

      /* Libere sous-arbre droit */
      if (huff->droite) {
	 libere_huffman (huff->droite);
      }

      free (huff);
      huff = NULL;
   }
}


int main(void)
{
   struct abr *ht = malloc (sizeof(struct abr));
   ht->est_feuille = false;
   ht->gauche = malloc (sizeof(struct abr));
   ht->gauche->est_feuille = true;
   ht->gauche->gauche = ht->gauche->droite = NULL;
   ht->gauche->sym = 0xc1;
   ht->droite = malloc (sizeof(struct abr));
   ht->droite->est_feuille = false;
   ht->droite->gauche = malloc (sizeof(struct abr));
   ht->droite->gauche->est_feuille = true;
   ht->droite->gauche->gauche = ht->droite->gauche->droite = NULL;
   ht->droite->gauche->sym = 0xea;
   ht->droite->droite = malloc (sizeof(struct abr));
   ht->droite->droite->est_feuille = true;
   ht->droite->droite->gauche = ht->droite->droite->droite = NULL;
   ht->droite->droite->sym = 0x33;

   affiche_huffman (ht);
   libere_huffman (ht);

   return 0;
}
