#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


void affiche_tab (uint8_t *tab, uint8_t h, uint8_t v){
   for (uint8_t i=0; i<v; i++){
      for (uint8_t j=0; j<h; j++){
	 printf("  %i  ",tab[i*h+j]);
      }
      printf("\n");
   }


}


/*bloc 4*4 -> bloc 4*4  */
void contraction_1_to_1(uint8_t *in, uint8_t *out){
      for (uint8_t i=0 ; i<16; i++){
	 out[i]=in[i];
      }
   }

   /* 2 bloc 4*4 -> bloc 4*2 */
   void contraction_2_to_1(uint8_t *in, uint8_t *out){

      uint8_t *temp=malloc(16*sizeof(uint8_t));

      /* pour chaque bloc, 4*4->4*2 */
      for (uint8_t i=0; i<32 ; i+=2){
	 temp[i/2]=0.5*(in[i]+in[i+1]);
      }

      /* Réordonnement: juxtaposition des deux blocs */

      for (uint8_t i=0; i<4; i++){
	 for (uint8_t j =0; j<2;j++) {
	    out[4*i+j]=temp[2*i+j];
	    out[4*i+j+2]=temp[2*i+8+j];
	 }

      }
      free(temp);

   };




/* 4 blocs 4*4 -> 4 blocs 2*2 -> 1 bloc 4*4 */
   void contraction_4_to_1(uint8_t *in, uint8_t *out){


      /* 4blocs 4*4 = 2*(2 blocs 4*4)  */

      uint8_t *aux1=malloc(32*sizeof(uint8_t));
      uint8_t *aux2=malloc(32*sizeof(uint8_t));

      for (uint8_t i=0; i<32; i++){
	    aux1[i]=in[i];
	    aux2[i]=in[i+32];
	 };


      printf(" \n TABLEAU DE BASE 4 BLOCS passés en 2 \n");
      printf("aux1 \n");
      affiche_tab(aux1, 4,8);
      printf("aux2 \n");
      affiche_tab(aux2, 4,8) ;

      uint8_t *temp1=malloc(16*sizeof(uint8_t));
      uint8_t *temp2=malloc(16*sizeof(uint8_t));

      contraction_2_to_1(aux1, temp1);
      contraction_2_to_1(aux2, temp2);

      printf(" \n CONTRACT LIGNE \n");
      affiche_tab(temp1, 4,4);
      printf("\n");
      affiche_tab(temp2, 4, 4);

      uint8_t *out1=malloc(8*sizeof(uint8_t));
      uint8_t *out2=malloc(8*sizeof(uint8_t));

      for (uint8_t i=0; i<4; i+=2){
	 for(uint8_t j=0; j<4; j++){
	    out1[j+4*i/2]=0.5*(temp1[j+4*i]+temp1[j+4*i+4]);
	     out2[j+4*i/2]=0.5*(temp2[j+4*i]+temp2[j+4*i+4]);
	 }
      }

      printf("CONTRACT COL \n");
      affiche_tab(out1, 4,2);
      printf("\n");
      affiche_tab(out2, 4,2);
      /*juxatposition verticale des deux blocs finaux*/
      for (uint8_t i=0; i<8; i++){
	 out[i]=out1[i];
	 out[i+8]=out2[i];

      }
      free(temp1);free(temp2); free(aux1); free(aux2); free(out1); free(out2);
   };




int main(void){

//  initialisation tableau

   uint8_t tab[16] ;
   for (uint8_t i =0; i<16; i++) {
      tab[i]=i;
   }

   uint8_t tab32[32];
    for (uint8_t i =0; i<16; i++) {
      tab32[i]=i;
      tab32[i+16]=i;
   }


    uint8_t tab64[64];

    for (uint8_t i=0; i<16; i++){
       tab64[i]=i;
       tab64[i+16]=i;
       tab64[i+32]=i;
       tab64[i+48]=i;
    }

   printf("Tab de base \n");
   affiche_tab(tab, 4,4) ;

   printf("Tab2 \n");
   affiche_tab(tab32, 4,8);

   uint8_t *out_11=malloc(8*sizeof(uint8_t));
   contraction_1_to_1(tab, out_11);
   printf("contraction 1->1 \n");
   affiche_tab(out_11, 4, 4);

   uint8_t*out_21=malloc(16*sizeof(uint8_t));
   contraction_2_to_1(tab32, out_21);
   printf("contraction 2->1 \n");
   affiche_tab(out_21, 4,4) ;

   uint8_t *out_41=malloc(16*sizeof(uint8_t));
   contraction_4_to_1(tab64, out_41);
   printf("contraction 4->1 \n");
   affiche_tab(out_41,4,4);


   free(out_11); free(out_21); free(out_41);
}
