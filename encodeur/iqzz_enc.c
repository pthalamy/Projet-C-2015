#include "iqzz_enc.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

void init_quantif(enum type_quantif q, uint8_t *quantif){

if (q==L){
quantif[0]=16;
quantif[1]=11;
quantif[2]=10;
quantif[3]=16;
quantif[4]=24;
quantif[5]=40;
quantif[6]=51;
quantif[7]=61;
quantif[8]=12;
quantif[9]=12;
quantif[10]=14;
quantif[11]=19;
quantif[12]=26;
quantif[13]=58;
quantif[14]=60;
quantif[15]=55;
quantif[16]=14;
quantif[17]=13;
quantif[18]=16;
quantif[19]=24;
quantif[20]=40;
quantif[21]=57;
quantif[22]=69;
quantif[23]=56;
quantif[24]=14;
quantif[25]=17;
quantif[26]=22;
quantif[27]=29;
quantif[28]=51;
quantif[29]=87;
quantif[30]=80;
quantif[31]=62;
quantif[32]=18;
quantif[33]=22;
quantif[34]=37;
quantif[35]=56;
quantif[36]=68;
quantif[37]=109;
quantif[38]=103;
quantif[39]=77;
quantif[40]=24;
quantif[41]=35;
quantif[42]=55;
quantif[43]=64;
quantif[44]=81;
quantif[45]=104;
quantif[46]=113;
quantif[47]=92;
quantif[48]=49;
quantif[49]=64;
quantif[50]=78;
quantif[51]=87;
quantif[52]=103;
quantif[53]=121;
quantif[54]=120;
quantif[55]=101;
quantif[56]=72;
quantif[57]=92;
quantif[58]=95;
quantif[59]=98;
quantif[60]=112;
quantif[61]=100;
quantif[62]=103;
quantif[63]=99;

}

else {

   for(uint8_t i=0 ; i<63; i++){
      quantif[i]=99;
   }
   quantif[0]=17;
   quantif[1]=18;
   quantif[2]=24;
   quantif[3]=47;
   quantif[8]=18;
   quantif[9]=21;
   quantif[10]=26;
   quantif[11]=66;
   quantif[16]=24;
   quantif[17]=26;
   quantif[18]=56;
   quantif[24]=47;
   quantif[25]=66;
}
}


/*Determine l'odre de parcours zigzag d'une matrice 8*8 */
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



void iqzz_enc(int32_t in[64], int32_t out[64],enum type_quantif q){

   uint32_t aux[64] ;
   uint8_t quantif[64];

   init_quantif(q, quantif);

   /* Division par la matrice de quantification */
   for (uint8_t k=0; k<64; k++){
      aux[k]=in[k]/quantif[k] ;
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
