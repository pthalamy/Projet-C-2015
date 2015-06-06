#include "dct.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#define N 8.
#define SQRT2 0.70710678118
#define COS(x, lambda) (cos(((2. * x + 1.) * lambda * acos(-1) / 16)))



double C(int32_t xi) {
   if (xi==0){
      return SQRT2 ;
   }
   else {
      return 1.;
   }
}


void dct(uint8_t in[64], int32_t out[64]){

   for (uint32_t u=0; u<8; u++){
      for (uint32_t v=0; v<8; v++){
	 double S=0 ;

	 for (uint32_t i=0; i<8; i++){
	    for (uint32_t j=0; j<8; j++){
	      S+= C(u)*C(v)*COS(i,u)*COS(j,v)*in[8*i+j];
	    }
	 }

	 S*=0.25;




	 out[8*u+v]=S;
      }
   }
}



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



void iqzz_enc(int32_t in[64], int32_t out[64], uint8_t quantif[64]){

   uint32_t aux[64] ;

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






void affiche_tab (int32_t *tab, uint8_t h, uint8_t v){
   for (uint8_t i=0; i<v; i++){
      for (uint8_t j=0; j<h; j++){
	 printf("  %i  ",tab[i*h+j]);
      }
      printf("\n");
   }


}

int main(void){

   uint8_t in[64];
   in[0]=0xa6;
   in[1]=0xa1;
   in[2]=0x9b;
   in[3]=0x9a;
   in[4]=0x9b;
   in[5]=0x9c;
   in[6]=0x97;
   in[7]=0x92;
   in[8]=0x9f;
   in[9]=0xa3;
   in[10]=0x9d;
   in[11]=0x8e;
   in[12]=0x89;
   in[13]=0x8f;
   in[14]=0x95;
   in[15]=0x94;
   in[16]=0xa5;
   in[17]=0x97;
   in[18]=0x96;
   in[19]=0xa1;
   in[20]=0x9e;
   in[21]=0x90;
   in[22]=0x90;
   in[23]=0x9e;
   in[24]=0xa7;
   in[25]=0x9b;
   in[26]=0x91;
   in[27]=0x91;
   in[28]=0x92;
   in[29]=0x91;
   in[30]=0x91;
   in[31]=0x94;
   in[32]=0xca;
   in[33]=0xda;
   in[34]=0xc8;
   in[35]=0x98;
   in[36]=0x85;
   in[37]=0x98;
   in[38]=0xa2;
   in[39]=0x96;
   in[40]=0xf0;
   in[41]=0xf7;
   in[42]=0xfb;
   in[43]=0xe8;
   in[44]=0xbd;
   in[45]=0x96;
   in[46]=0x90;
   in[47]=0x9d;
   in[48]=0xe9;
   in[49]=0xe0;
   in[50]=0xf1;
   in[51]=0xff;
   in[52]=0xef;
   in[53]=0xad;
   in[54]=0x8a;
   in[55]=0x90;
   in[56]=0xe7;
   in[57]=0xf2;
   in[58]=0xf1;
   in[59]=0xeb;
   in[60]=0xf7;
   in[61]=0xfb;
   in[62]=0xd0;
   in[63]=0x97;


   uint8_t in2[64];
   in2[0]=139;
   in2[1]=144;
   in2[2]=149;
   in2[3]=153;
   in2[4]=155;
   in2[5]=155;
   in2[6]=155;
   in2[7]=155;
   in2[8]=144;
   in2[9]=151;
   in2[10]=153;
   in2[11]=156;
   in2[12]=159;
   in2[13]=156;
   in2[14]=156;
   in2[15]=156;
   in2[16]=150;
   in2[17]=155;
   in2[18]=160;
   in2[19]=163;
   in2[20]=158;
   in2[21]=156;
   in2[22]=156;
   in2[23]=156;
   in2[24]=159;
   in2[25]=161;
   in2[26]=162;
   in2[27]=160;
   in2[28]=160;
   in2[29]=159;
   in2[30]=159;
   in2[31]=159;
   in2[32]=159;
   in2[33]=160;
   in2[34]=161;
   in2[35]=162;
   in2[36]=162;
   in2[37]=155;
   in2[38]=155;
   in2[39]=155;
   in2[40]=161;
   in2[41]=161;
   in2[42]=161;
   in2[43]=161;
   in2[44]=160;
   in2[45]=157;
   in2[46]=157;
   in2[47]=157;
   in2[48]=162;
   in2[49]=162;
   in2[50]=161;
   in2[51]=163;
   in2[52]=162;
   in2[53]=157;
   in2[54]=157;
   in2[55]=157;
   in2[56]=162;
   in2[57]=162;
   in2[58]=161;
   in2[59]=161;
   in2[60]=163;
   in2[61]=158;
   in2[62]=158;
   in2[63]=158;


   uint8_t quantif[64] ;

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



   int32_t out[64] ;
   dct(in, out);
   affiche_tab(out, 8,8);

   printf("\n \n");
   int32_t out2[64];
   dct(in2,out2);
   affiche_tab(out2,8,8);

   printf ("\n\n");
   int32_t out3[64] ;
   iqzz_enc(out2, out3, quantif);
   affiche_tab(out3, 8,8);

return 0 ;

}
