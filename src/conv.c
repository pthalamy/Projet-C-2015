#include "conv.h"
#include <stdio.h>		/* DEBUG PURPOSE */

#define MCU_BASE 8
#define UINT8_FIRST 0
#define UINT8_LAST 255

enum RGB {
   R,
   G,
   B
};

uint8_t toRGB(enum RGB c, uint8_t y, uint8_t Cb, uint8_t Cr)
{
   int16_t res_16 = 0;

   /* Calcul de la valeur RGB correspondante */
   switch (c) {
   case R :
      res_16 = y - 0.0009267 * (Cb - 128) + 1.4016868 * (Cr - 128);
      break;
   case G:
      res_16 = y - 0.3436954 * (Cb - 128) - 0.7141690 * (Cr - 128);
      break;
   case B:
      res_16 = y + 1.7721604 * (Cb - 128) + 0.0009902 * (Cr - 128);
      break;
   }

   /* Detection et correction de la saturation */
   uint8_t res_8;
   if (res_16 > UINT8_LAST) {
      res_8 = 255;
   } else if (res_16 < UINT8_FIRST) {
      res_8 = 0;
   } else
      res_8 = res_16;

   return res_8;
}

void YCbCr_to_ARGB(uint8_t  *mcu_YCbCr[3], uint32_t *mcu_RGB,
		   uint32_t nb_blocks_h, uint32_t nb_blocks_v)
{
   /* Pour chaque pixel des MCU YCbCr, passage en ARGB */
   for (uint32_t i = 0;
	i < (MCU_BASE * nb_blocks_v) * (MCU_BASE * nb_blocks_h); /* Nb elts mcu */
	i++) {
      mcu_RGB[i]  = 0;	/* Mise à 0, nécessaire pour A */
      mcu_RGB[i] |=
	 toRGB (R, mcu_YCbCr[0][i], mcu_YCbCr[1][i], mcu_YCbCr[2][i]) << 16;
      mcu_RGB[i] |=
	 toRGB (G, mcu_YCbCr[0][i], mcu_YCbCr[1][i], mcu_YCbCr[2][i]) << 8;
      mcu_RGB[i] |=
	 toRGB (B, mcu_YCbCr[0][i], mcu_YCbCr[1][i], mcu_YCbCr[2][i]);
   }
}
