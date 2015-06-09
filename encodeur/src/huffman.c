#include "huffman.h"



/*Recherche un symbole dans un tableau tab[256] */
/*renvoie -1 si l'element n'y est pas, i si tab[i].abr->abr->symbole =symb  */

int32_t recherche_tab(struct elt *tab[256], uint8_t symb ){
   bool trouve=false ;
   int32_t i  =0 ;
   while ((! trouve) & (i<256)){
      if (tab[i]->abr->symbole==symb) {
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
void init_freq(int32_t bloc[64],
	       struct elt *freq_DC[256], uint8_t ind_DC,
	       struct elt *freq_AC[256], uint8_t ind_AC,
	       int32_t  *pred_DC){

/* Maj de freq_DC */

   int32_t dc= bloc[0]-*pred_DC ;
   uint8_t mag = magnitude(dc);

   if (recherche_tab( freq_DC, (uint8_t)dc) == -1 ){
      freq_DC[ind_DC]->abr->symbole=dc ;
      freq_DC[ind_DC]->occ =1 ;
      ind_DC ++;
   } else {
      freq_DC[recherche_tab(freq_DC, (uint8_t)dc)]->occ++ ;
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
	 freq_AC[ind_AC]->abr->symbole=val ;
	 freq_AC[ind_AC]->occ= 1 ;
	 ind_AC++;
      } else {
	 freq_AC[(uint8_t)recherche_tab(freq_AC, val)]->occ++;
      }
   }
}


/////////////////////////////////////////////////////////////////
////////    FONCTIONS SUR LES TAS /////////////

/*Echange deux éléments d'un tas*/
void swap_heap(struct elt *a, struct elt *b){
   struct elt *temp=malloc (sizeof(struct elt));
   temp->abr=malloc(sizeof(struct abr));
   temp->occ= a->occ;
   temp->abr->symbole=a->abr->symbole ;
   a->occ=b->occ ;
   a->abr->symbole=b->abr->symbole ;
   b->occ=temp->occ;
   b->abr->symbole=temp->abr->symbole ;
   free(temp->abr);
   free(temp);
}

/*Insertion d'un élément dans le tas */

void insert_heap(struct elt *x, struct elt *heap,
		 uint8_t ind, uint8_t taille_heap){

   uint8_t i =ind ;
   if (ind >taille_heap){
      printf("Tas plein !");
   };

   /*insertion à la fin du tas*/
   heap[ind].abr=malloc(sizeof(struct abr));
   heap[ind].abr->est_feuille=true ;
   heap[ind].abr->symbole=x->abr ->symbole ;
   heap[ind].occ= x->occ ;
   ind++;// ind est l'indice de la 1e case vide du tas


   /*tant que le champ occ du père est supérieur à celui de x on les échange */
   while (i>0){
      if (heap[i].occ < heap[i/2].occ){
	 swap_heap(&heap[i], &heap[i/2]);
	 i=i/2 ;
      } else {
	 i=0;
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
void delete_elt(struct elt *heap, uint8_t *ind){

   if (*ind==0){
      printf("Tas vide :impossible de supprimer un élément ");
   };

   /* Derniere feuille passe en racine */
   swap_heap(&heap[0], &heap[*ind-1]) ;
   heap[*ind-1].occ=0 ;
   (*ind)--;

   /*Tant qu'on n'est pas sur une feuille */
   /* si un des fils a une occurrence plus faible que le pere, on les echange */

   uint8_t i=0 ;
   uint8_t min ;
   /* Calcul du fils avec la plus faible occurrence */
   while (i<(*ind-1)/2){
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


struct elt *tab_to_heap(struct elt *tab[256], uint8_t *nb_elt ){

   /* Création d'un tas de bonne taille */
   int32_t i =0 ;
   bool fin_tab=false ;

   while ((! fin_tab)&& (i<256)) {

      if (tab[i]==NULL){
	 fin_tab=true;

      } else {
	 i++;
      }

      *nb_elt=i;
   }



   /*Allocation du tas */
   struct elt *heap=smalloc(*nb_elt*sizeof(struct elt));


   /* Remplissage du tas */
   for (uint8_t j=0; j<*nb_elt; j++){
      insert_heap(tab[j], heap, j, *nb_elt) ;

   }

   return heap ;
}



/*Libération d'un tas*/
void free_heap( struct elt *heap){
   free(heap);
}


////////////////////////////////////////////////////////////////////
///////////   FONCTIONS SUR LES ARBRES DE HUFFMAN /////////


struct huff_table {
   struct abr *huff_tree ;
};




struct abr *create_huffman_table(struct elt tab[256], uint8_t *nb_elt)
{

   struct abr *gauche ;
   struct abr *droit ;
   uint8_t sum_occ ;
   struct elt *pere ;

   /*Transformation du tableau en tas */
   struct elt *heap=tab_to_heap( &tab,  nb_elt);

   /*Tant que il reste des elt a traiter*/
   while (*nb_elt>0){
      sum_occ = 0 ;

      /*Fusion des deux meilleurs noeuds en un arbre*/

      gauche=best_elt(heap, *nb_elt).abr;
      sum_occ=best_elt(heap, *nb_elt).occ ;
      delete_elt(heap, nb_elt);
      (*nb_elt) --;


      droit =best_elt(heap, *nb_elt).abr;
      sum_occ=best_elt(heap, *nb_elt).occ;
      delete_elt(heap, nb_elt);
      *(nb_elt) --;

      pere = smalloc(sizeof(struct elt));
      pere->abr->gauche=gauche ;
      pere->abr->droit=droit ;
      pere->occ=sum_occ ;
      pere->abr->est_feuille=false ;

      insert_heap(pere, heap, *nb_elt, *nb_elt);
      (*nb_elt) ++;



   }
   /*Désallocation*/
   free_heap(heap);

   return (heap[0].abr);
}


void huffman_value(struct abr *abr, uint8_t symbole) {



}



void store_huffman_table(struct bitstream *stream, struct huff_table *ht)
{

}

void free_huffman_table(struct huff_table *table)
{

}
