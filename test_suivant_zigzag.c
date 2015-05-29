#include <stdio.h>
#include <stdint.h>

int32_t suivant_zigzag(int32_t *i,int32_t *j){


// si i+j est pair, on est en train de remonter dans la matrice
   if ((*i+*j)%2==0){

      // si on ne peut plus monter
      if (*i==0){
	 // si on peut, on part à droite
	 if (*j!=3){
	    (*j)++;
	 }
	 // sinon on descend
	 else{
	    (*i)++;
	       }
      }

      // si on est tout à droite, on descend
      else if (*j==3){
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
	 if (*i !=3){
	    (*i)++;
	 }
	 // sinon on part à droite
	 else {
	    (*j)++;
	 }
      }

      // si on ne peut plus descendre
      else if (*i==3){
	 (*j)++;
      }
      // sinon on continue sur la diagonale descendante
      else {
	 (*j)--;
	 (*i)++;
      }
   }

   return (4*(*i)+*j);
}

void iqzz_block (int32_t in[16], int32_t out[16], uint8_t quantif[16])
{
   int32_t n=0;
   int32_t i;
   int32_t j;



   for (uint32_t m=0; m<16; m++) {
      out[m]=in[n]*quantif[n];
      i=n/4;
      j=n%4;
      n=suivant_zigzag(&i, &j);
      printf("%i \n ", n);
   }

}


int  main(void){
   int32_t in[16];
   int32_t out[16];
   uint8_t quantif[16];

   iqzz_block(in, out, quantif);
   return 0;

}
