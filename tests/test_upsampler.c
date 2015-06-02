//#include "upsampler.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// prérequis: bloc de taille 16
uint8_t *dilatation_ligne(uint8_t *bloc, uint8_t *out){
      for(uint8_t i=0; i<32; i++){
      out[i]=bloc[i/2];
   }
   return  out ;
}


// prérequis : bloc de taille 16 (4*4)
uint8_t *dilatation_lc(uint8_t *bloc, uint8_t *out){

   uint8_t *temp=malloc(32*sizeof(uint8_t));

   temp=dilatation_ligne(bloc, temp);


   for (uint8_t i=0; i<8; i++){
      for(uint8_t j=0; j<8; j++){
	 out[j+8*i]=temp[i/2*8+j];

      }
   }
   free(temp);
   return out ;
}

// 2 blocs de taille 16(4*4) -> un bloc de taille 32(4*8)
uint8_t *juxtaposition_horizontale(uint8_t *bloc, uint8_t *out){



   for (uint8_t i=0; i<4; i++){
      for (uint8_t j=0 ; j<4 ; j++) {
	 out[8*i+j]=bloc[i*4+j];

	 out[j+4 +i*8]=bloc[i*4+16+j];
      }
   }
   return out ;
};

void affiche_tab (uint8_t *tab, uint8_t h, uint8_t v){
   for (uint8_t i=0; i<v; i++){
      for (uint8_t j=0; j<h; j++){
	 printf("  %i  ",tab[i*h+j]);
      }
      printf("\n");
   }


}



//4blocs de taille 16(4*4) --> un bloc de taille (8*8)
uint8_t *juxtaposition_hv(uint8_t *bloc, uint8_t *out){

   uint8_t *temp1=malloc(32*sizeof(uint8_t));
   uint8_t *temp1bis=malloc(32*sizeof(uint8_t));
   uint8_t *temp2=malloc(32*sizeof(uint8_t));
   uint8_t *temp2bis=malloc(32*sizeof(uint8_t));


   for(uint8_t i=0; i<32; i++){
      temp1[i]=bloc[i];
      temp1bis[i]=bloc[i];
      temp2[i]=bloc[i+32];
      temp2bis[i]=bloc[i+32];
   }


   printf("temp1 \n");
   affiche_tab(temp1, 4, 8);

   printf(" \n temp2 \n ");
   affiche_tab(temp2, 4, 8);

   temp1=juxtaposition_horizontale(temp1, temp1bis);
   temp2=juxtaposition_horizontale(temp2, temp2bis);

    printf("temp1 juxtap \n");
   affiche_tab(temp1, 8, 4);

   printf(" \n temp2 juxtap \n ");
   affiche_tab(temp2, 8, 4);



   printf("fin temp \n");
   for (uint8_t i=0; i<32; i++){
      out[i]=temp1[i];
      out[i+32]=temp2[i];
   }
   free(temp1);
   free(temp2);
   free(temp1bis);
   free(temp2bis);
   return out;
}




int  main(void){

//  initialisation tableau

   uint8_t tab[16] ;
   for (uint8_t i =0; i<16; i++) {
      tab[i]=i;
   }

   uint8_t *out=malloc(32*sizeof(uint8_t));
   affiche_tab(tab, 4, 4);
   printf("dilatation ligne \n");
   affiche_tab(dilatation_ligne(tab, out), 8,4);

   uint8_t*out2=malloc(64*sizeof(uint8_t));
   printf("dilatation lc \n");
   affiche_tab(dilatation_lc(tab, out2), 8, 8);
   printf("\n \n");
   printf("tab bis \n");

   uint8_t tab2[32] ;
   for (uint8_t i=0; i<16; i++){
      tab2[i]=i ;
      tab2[i+16]=i;
   }


   affiche_tab(tab2,4,8);

   uint8_t *out3=malloc(32*sizeof(uint8_t));
   printf("juxtaposition horizontale \n") ;
   affiche_tab(juxtaposition_horizontale(tab2, out3), 8,4);

   printf("\n \n");
   printf("tab3 \n ");

   uint8_t tab3[64];
   for(uint8_t i=0; i<16; i++){
      tab3[i]=i;
      tab3[i+16]=i;
      tab3[i+32]=i+100;
      tab3[i+48]=i+100;
   }
   affiche_tab(tab3, 4,16);

   printf("\n juxta hv \n");
   uint8_t *out4=malloc(64*sizeof(uint8_t));
   affiche_tab(juxtaposition_hv(tab3, out4) , 8 , 8);

   free(out); free(out2); free(out3);free(out4);

   return 0;
}
