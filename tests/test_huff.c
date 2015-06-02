
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
      uint8_t code[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
      uint8_t nbits = 0;
      affiche_huffman_rec (huff, code, nbits);
   }
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
	 (*abr)->sym = 0x0;
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

void rec_parcours_abr(struct abr *arbre, char *code, uint8_t c, uint8_t *symbole) {
   if (!arbre) {
      if (arbre->est_feuille) {
	 *symbole = arbre->sym;
	 return ;
      }

      if (code[c] == '1') {
	 c++;
	 rec_parcours_abr(arbre->droite, code, c, symbole);
      } else if (code[c] == '0') {
	 c++;
	 rec_parcours_abr(arbre->gauche, code, c, symbole);
      } else {
	 printf("erreur dans la lecture bit !=0ou1\n");
	 exit(1);
      }

   }
}

int8_t next_huffman_value(struct abr *table, char *code)
{
   char *direction = code;
   uint8_t symb;
   rec_parcours_abr (table, direction, 0, &symb);

   return symb;
}

int main(void)
{
   struct abr *ht = NULL;

   printf ("INSERTIONS\n");
   insertion (&ht, 2, 0x0d);
   insertion (&ht, 2, 0x0e);
   insertion (&ht, 2, 0x0a);
   insertion (&ht, 3, 0x0c);
   insertion (&ht, 3, 0x0b);

   affiche_huffman (ht);

   printf ("\nNEXT_HUFFMAN_VALUE\n");
   char *code = "00";
   printf ("%s <=> 0x%x\n", code, next_huffman_value  (ht, code));
   code = "01";
   printf ("%s <=> 0x%x\n", code, next_huffman_value  (ht, code));
   code = "10";
   printf ("%s <=> 0x%x\n", code, next_huffman_value  (ht, code));
   code = "110";
   printf ("%s <=> 0x%x\n", code, next_huffman_value  (ht, code));
   code = "111";
   printf ("%s <=> 0x%x\n", code, next_huffman_value  (ht, code));


   libere_huffman (ht);

   return 0;
}
