#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

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




/* 4 blocs 4*4 -> 4 blocs 2*2 */
   void contraction_4_to_1(uint8_t *in, uint8_t *out){

      uint8_t *temp=malloc(16*sizeof(uint8_t));

      /* on sépare les 4 blocs en 2*2 blocs pour les passer en paramètres de  contraction 2 to 1 */


      contraction_2_to_1(in ,temp);



      for (uint8_t i=0; i<4; i+=2){
	 for(uint8_t j=0; j<2; j++){
	    out[j+2*i/2]=(uint8_t)(0.5*(temp[j+2*i]+temp[j+2*i+2]));
	 }
      }
      free(temp);
   };

void affiche_tab (uint8_t *tab, uint8_t h, uint8_t v){
   for (uint8_t i=0; i<v; i++){
      for (uint8_t j=0; j<h; j++){
	 printf("  %i  ",tab[i*h+j]);
      }
      printf("\n");
   }


}


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

   uint8_t *out_41=malloc(4*sizeof(uint8_t));
   contraction_4_to_1(tab, out_41);
   printf("contraction 4->1 \n");
   affiche_tab(out_41,2,2);


   free(out_11); free(out_21); free(out_41);
}
