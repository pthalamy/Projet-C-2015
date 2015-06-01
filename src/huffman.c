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



// insertion à la profondeur prof, le plus a gauche possible
// renvoie une booléen qui permet de savoir si l'insertion a réussi
bool insertion_gauche( struct abr *abr, uint8_t prof, uint8_t symbole){

   // insertion juste en dessous de la racine
   if (prof==1){
      if (abr==NULL){
	 printf("abr null \n ");
	 return false ;
      }
      if (abr->gauche==NULL){
	 abr->gauche= malloc(sizeof(struct abr));
	 abr -> gauche ->sym = symbole ;
	 return true;
      }
      else if (abr->droite ==NULL){
	 abr -> droite= malloc (sizeof (struct abr));
	 abr->droite ->sym= symbole ;
	 return true;
      }
      else {
	 printf("niveau plein \n ");
	 return false ;
      }
   }

   else {
      if (insertion_gauche(abr->gauche, prof-1, symbole)){
	 return true; }

      else if (insertion_gauche(abr->droite, prof-1, symbole)){
	 return true;
      }
      else {
	 return false ;
      }
   }

}



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
   printf("alloc huff table");
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

// Construction de l'arbre

   //Calcul de la longueur maximale
   uint8_t lg_max ;
   uint8_t i=15 ;
   while(symbOfLen[i]==0){
      i--;
   }
   lg_max =i;

   //Remplissage de l'arbre niveau par niveau

   uint8_t compteur_symbole; // nb de symboles insérés sur une prof
   uint8_t compteur_table=0; // nb de symboles a insérer sur une prof

   uint8_t nb_symb_codes=0;

   for (uint8_t j =0; j< lg_max; j++ ){
      compteur_table= symbOfLen[j];
      compteur_symbole=0;

      while(compteur_symbole !=compteur_table){
	bool  insert= insertion_gauche(huff->huff_tree, j+1,symboles[nb_symb_codes]);

	if (insert) {
	compteur_symbole++;
	 nb_symb_codes++;
      }


   }



}
   return huff;
}

void rec_parcours_abr(struct abr *arbre, uint32_t direction, uint8_t *symbole, struct bitstream *stream){

   bool byte_stuffing= false;

   if (arbre==NULL) {
      printf("arbre null");
      return ;
   }
   else if ((arbre -> gauche ==NULL)&(arbre->droite ==NULL)){
      *symbole = arbre -> sym;
      printf("feuille atteinte");
      return ;
   }
   else {
      uint8_t suivant=read_bitstream(stream, 1, &direction, byte_stuffing);
      (void) suivant;

      if (direction==1){
	 rec_parcours_abr(arbre->gauche,direction , symbole, stream);
      }
      else if (direction==0){
	 rec_parcours_abr(arbre->droite,direction, symbole, stream);
      }
      else {
	 printf("erreur dans la lecture bit !=0ou1");
	 exit(1);
      }
   }

}



int8_t next_huffman_value(struct huff_table *table,
			  struct bitstream *stream)
{
   uint32_t direction;
   uint8_t symb ;
   uint8_t suiv=read_bitstream(stream, 1, &direction, false);
   (void)suiv;
   rec_parcours_abr(table->huff_tree, direction, &symb,  stream );
   return symb;

}

void free_huffman_table(struct huff_table *table)
{

}
