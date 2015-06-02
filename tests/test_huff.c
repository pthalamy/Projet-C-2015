
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



// insertion à la profondeur prof, le plus a gauche possible
// renvoie une booléen qui permet de savoir si l'insertion a réussi
// prérequis : profondeur inf ou égale à lg max !!!!
bool insertion_gauche( struct abr *abr, uint8_t prof, uint8_t symbole){


   if (prof==1){
      printf("prof=1 \n");
      if(abr==NULL){
	 abr=malloc(sizeof(struct abr));
	 insertion_gauche(abr, prof, symbole);
	 abr->est_feuille=false;
	 return true;
      }
      else if (abr->gauche ==NULL){
	 abr->gauche= malloc(sizeof(struct abr));
	 printf("arbre alloué g \n");
	 abr->gauche->sym=symbole;
	 abr->gauche->est_feuille=true;
	 return true ;
      }
      else if (abr->droite ==NULL){
	 abr->droite=malloc(sizeof(struct abr));
	 printf("arbre alloué d \n ");
	 abr->droite->sym=symbole;
	 abr->droite->est_feuille =true;
	 return true ;
      }
      else {
	 printf ("niveau plein");
	 return(false);
      }
      printf("sortie boucle1 \n");
   }

   else {
      printf ("prof diff1 \n");
      if (abr==NULL){
	 printf("alloc nv noeud non feuille \n");
	 abr=malloc(sizeof(struct abr));
	 insertion_gauche(abr, prof, symbole);
      }
      else {
	 printf("acs arbre non null \n");
	 if (insertion_gauche(abr->gauche, prof-1, symbole)){
	    printf("appel gauche \n");
	    return true; }

	 else if (insertion_gauche(abr->droite, prof-1, symbole)){
	    printf("appel droite \n");
	    return true;
	 }
	 else {
	    printf("pas de place pour descendre");
	    return false ;
	 }
      }

   }

}
   int main(void)
   {
      struct abr *ht = malloc (sizeof(struct abr));
      ht->est_feuille = false;
      //ht->gauche = malloc (sizeof(struct abr));
      //ht->gauche->est_feuille = true;
      //ht->gauche->gauche = ht->gauche->droite = NULL;
      //ht->gauche->sym = 0xc1;
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
      printf("insertion \n");
      insertion_gauche(ht, 2, 0x12);
      affiche_huffman(ht);
      libere_huffman (ht);

      return 0;
   }
