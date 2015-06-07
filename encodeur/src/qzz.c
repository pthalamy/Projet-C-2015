#include "qzz.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

uint8_t table_quantif[2][64] = {
   {
      16, 11, 10, 16, 24, 40, 51, 61,
      12, 12, 14, 19, 26, 58, 60, 55,
      14, 13, 16, 24, 40, 57, 69, 56,
      14, 17, 22, 29, 51, 87, 80, 62,
      18, 22, 37, 56, 68, 109, 103, 77,
      24, 35, 55, 64, 81, 104, 113, 92,
      49, 64, 78, 87, 103, 121, 120, 101,
      72, 92, 95, 98, 112, 100, 103, 99
   },  {
      17, 18, 24, 47, 99, 99, 99, 99,
      18, 21, 26, 66, 99, 99, 99, 99,
      24, 26, 56, 99, 99, 99, 99, 99,
      47, 66, 99, 99, 99, 99, 99, 99,
      99, 99, 99, 99, 99, 99, 99, 99,
      99, 99, 99, 99, 99, 99, 99, 99,
      99, 99, 99, 99, 99, 99, 99, 99,
      99, 99, 99, 99, 99, 99, 99, 99
   } };

/*Determine l'odre de parcours zigzag d'une matrice 8*8 */
int32_t suivant_zigzag(int32_t *i,int32_t *j)
{
// si i+j est pair, on est en train de remonter dans la matrice
   if ((*i+*j)%2==0){

      // si on ne peut plus monter
      if (*i==0){
	 // si on peut, on part à droite
	 if (*j!=7){
	    (*j)++;
	 }
	 // sinon on descend
	 else{
	    (*i)++;
	 }
      }

      // si on est tout à droite, on descend
      else if (*j==7){
	 (*i)++;
      }
      // sinon on remonte sur la diagonale
      else {
	 (*j)++;
	 (*i)--;
      }
   }

// sinon on est en descente dans la matrice
   else {
      // si on ne peut pas aller plus à gauche
      if (*j==0){
	 //si on peut descendre on le fait
	 if (*i !=7){
	    (*i)++;
	 }
	 // sinon on part à droite
	 else {
	    (*j)++;
	 }
      }

      // si on ne peut plus descendre
      else if (*i==7){
	 (*j)++;
      }
      // sinon on continue sur la diagonale descendante
      else {
	 (*j)--;
	 (*i)++;
      }
   }
   return (8*(*i)+*j);
}



void iqzz_enc(int32_t in[64], int32_t out[64],enum type_quantif q)
{
   uint32_t aux[64] ;

   /* Division par la matrice de quantification */
   for (uint8_t k=0; k<64; k++){
      aux[k]=in[k] / table_quantif[q][k] ;
   }


   /* Réorganisation zigzag */

   uint8_t n=0;
   int32_t i;
   int32_t j ;

   for(uint8_t k=0; k<64; k++){
      out[k]=aux[n];
      i=n/8 ;
      j=n%8;
      n=suivant_zigzag(&i, &j);

   }

}
