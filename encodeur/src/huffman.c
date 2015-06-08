#include "pack.h"
#include "huffman.h"
#include <stdio.h>
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



/*Parcours d'un bloc et stockage des symboles et de leurs ocurrences dans un tableau*/

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
