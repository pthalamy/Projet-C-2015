#include "pack.h"
#include "huffman.h"
#include <stdio.h>
#include <stdlib.h>
struct elt {
   uint8_t symbole ;
   uint8_t occ ;

};

/*Recherche un symbole dans un tableau tab[256] */
/*renvoie -1 si l'element n'y est pas, i si tab[i].symbole =symb  */

int32_t recherche_tab(struct elt tab[256], uint8_t symb ){
   bool trouve=false ;
   int32_t i  =0 ;
   while ((! trouve) & (i<256)){
      if (tab[i].symbole==symb) {
	 trouve=true;
      } else {
	 i++;
      }

   }
   if (trouve){
      return i;
   } else {
      return -1 ;
   }

}



/*Parcours d'un bloc et stockage des symboles et de leurs ocurrences dans un tableau AC et un tableau DC */
void init_freq(int32_t bloc[64], struct elt freq_DC[256], struct elt freq_AC[256], int32_t  *pred_DC, uint8_t ind_DC, uint8_t ind_AC){

/* Maj de freq_DC */

   int32_t dc= bloc[0]-*pred_DC ;
   uint8_t mag = magnitude(dc);

   if (recherche_tab( freq_DC, (uint8_t)dc) == -1 ){
      freq_DC[ind_DC].symbole=dc ;
      freq_DC[ind_DC].occ =1 ;
      ind_DC ++;
   } else {
      freq_DC[recherche_tab(freq_DC, (uint8_t)dc)].occ++ ;
   }

   *pred_DC =dc ;

/* Maj de freq_AC */

   uint8_t i=1 ;
   uint32_t nb_zeros=0;
   uint8_t val ;
   uint8_t der =1;

   while (i<63){

      // si on a un zéro on passe au coeff suivant en incrémentant le compteur
      if (bloc[i]==0){
	 nb_zeros++;
	 i++;


      } else {
	 if (nb_zeros>16){
	    printf("erreur: format incorrect");
	 }

	 /*calcul du symbole (nb zeros + mag) */
	 mag=magnitude(bloc[i]);
	 val= (nb_zeros <<4)||(0x0F & mag);

	 /*maj indices*/
	 der=i;
	 i++;
	 nb_zeros=0;

      }

      if (der !=63){
	 val=0x00;
      }

      /*Maj de freq_AC*/
      if (recherche_tab(freq_AC, val)==-1){
	 freq_AC[ind_AC].symbole=val ;
	 freq_AC[ind_AC].occ= 1 ;
	 ind_AC++;
      } else {
	 freq_AC[(uint8_t)recherche_tab(freq_AC, val)].occ++;
      }
   }
}


/*Echange deux éléments d'un tas*/
void swap_heap(struct elt *a, struct elt *b){
   struct elt *temp=malloc (sizeof(struct elt));
   temp->occ= a->occ;
   temp->symbole=a->symbole ;
   a->occ=b->occ ;
   a->symbole=b->symbole ;
   b->occ=temp->occ;
   b->symbole=temp->symbole ;

   free(temp);
}

/*Insertion d'un élément dans le tas */
void insert_heap(struct elt x, struct elt *heap,
		 uint8_t ind, uint8_t taille_heap){

   uint8_t i =ind ;
   if (ind >taille_heap){
      printf("tas plein !");
   };

   /*insertion à la fin du tas*/
   heap[ind].symbole=x.symbole ;
   heap[ind].occ= x.occ ;
   ind++;// ind est l'indice de la 1e case vide du tas

   /*tant que le champ occ du père est supérieur à celui de x on les échange */
   while (i>0){
      if (heap[i].occ < heap[i/2].occ){
	 swap_heap(&heap[i], &heap[i/2]);
	 i=i/2 ;
      }
   }
}






/*Recupère l'élément de plus faible occurrence*/
struct elt best_elt(struct elt *heap, uint8_t ind){

   if (ind==0){
      printf("tas vide \n");
   };

   return heap[0];


}

/*Supprime l'élément de plus faible occurrence */
void delete_elt(struct elt *heap, uint8_t ind){

   if (ind==0){
      printf("Tas vide :impossible de supprimer un élément ");
   };

   /* Derniere feuille passe en racine */
   swap_heap(&heap[0], &heap[ind-1]) ;
   heap[ind-1].occ=0 ;
   ind--;

   /*Tant qu'on n'est pas sur une feuille */
   /* si un des fils a une occurrence plus faible que le pere, on les echange */

   uint8_t i=0 ;
   uint8_t min ;
   /* Calcul du fils avec la plus faible occurrence */
   while (i<(ind-1)/2){
      if (heap[2*i].occ > heap[2*i+1].occ){
	 min=2*i+1;
      } else {
	 min=2*i ;
      }

      if (heap[i].occ>heap[min].occ){
	 swap_heap(&heap[i], &heap[min]);
	 i=min;
      }
   }

}

/*Transformation du tableau en file de priorité */
struct elt *tab_to_heap(struct elt tab[256], uint8_t *nb_elt ){

  /* Création d'un tas de bonne taille */
  int32_t i =0 ;
   bool fin_tab=false ;
   while ((! fin_tab)& (i<256)) {
      if (tab[i].occ==0){
	 fin_tab=true;
      } else {
	 i++;
      }

      *nb_elt=i+1;
   }

   /*Allocation du tas */
   struct elt *heap=malloc(*nb_elt*sizeof(struct elt));
   //smalloc();


   return heap ;
}


/*Libération d'un tas*/
void free_heap( struct elt *heap){
   free(heap);
}






struct huff_table {

};

/* extern struct huff_table create_huffman_table(...)
   {

   } */

void store_huffman_table(struct bitstream *stream, struct huff_table *ht)
{

}

void free_huffman_table(struct huff_table *table)
{

}
