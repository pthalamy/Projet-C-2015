//#include "upsampler.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// prérequis: bloc de taille 16
uint8_t *dilatation_ligne(uint8_t *bloc){
   uint8_t *new_bloc=malloc(32*sizeof(uint8_t));
   for(uint8_t i=0; i<32; i++){
      new_bloc[i]=bloc[i/2];
   }
   return  new_bloc ;
}


// prérequis : bloc de taille 32 ( 4*8 )
uint8_t *dilatation_colonne(uint8_t *bloc){
   uint8_t * new_bloc=malloc(64*sizeof(uint8_t));


   for (uint8_t i=0; i<8; i++){
      for(uint8_t j=0; j<8; j++){
	 new_bloc[j+8*i]=bloc[i/2*8+j];

      }
   }
   return new_bloc ;
}

// 2 blocs de taille 16(4*4) -> un bloc de taille 32(4*8)
uint8_t *juxtaposition_horizontale(uint8_t *bloc){

   uint8_t *new_bloc=malloc(32*sizeof(uint8_t));

   for (uint8_t i=0; i<4; i++){
      for (uint8_t j=0 ; j<4 ; j++) {
	 new_bloc[8*i+j]=bloc[i*4+j];

	 new_bloc[j+4 +i*8]=bloc[i*4+16+j];
      }
   }
   return new_bloc ;
};

void affiche_tab (uint8_t *tab, uint8_t h, uint8_t v){
   for (uint8_t i=0; i<v; i++){
      for (uint8_t j=0; j<h; j++){
	 printf("  %i  ",tab[i*h+j]);
      }
      printf("\n");
   }


}

int  main(void){

//  initialisation tableau

   uint8_t tab[16] ;
   for (uint8_t i =0; i<16; i++) {
      tab[i]=i;
   }
   affiche_tab(tab, 4, 4);
   printf("dilatation ligne \n");
   affiche_tab(dilatation_ligne(tab), 8,4);

   printf("dilatation colonne \n");
   affiche_tab(dilatation_colonne(dilatation_ligne(tab)), 8, 8);
   printf("\n \n");
   printf("tab bis \n");

   uint8_t tab2[32] ;
   for (uint8_t i=0; i<16; i++){
      tab2[i]=i ;
      tab2[i+16]=i;
   }

   affiche_tab(tab2,4,8);

   printf("juxtaposition horizontale \n") ;
   affiche_tab(juxtaposition_horizontale(tab2), 8,4);
   return 0;
}
