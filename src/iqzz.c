#include "iqzz.h"

#include <stdio.h>

int32_t suivant_zigzag(int32_t *i,int32_t *j){

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

void iqzz_block (int32_t in[64], int32_t out[64], uint8_t quantif[64])
{
   int32_t n=0;
   int32_t i;
   int32_t j;

        /* for (n=0; n=63; n=suivant_zigzag(n/8, n%8)){ */

   /* } */

   for (uint32_t m=0; m<64; m++) {
      out[n]=in[m]*quantif[m];
      i=n/8;
      j=n%8;
      n=suivant_zigzag(&i, &j);

   }

}
