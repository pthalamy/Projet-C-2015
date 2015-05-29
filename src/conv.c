#include "conv.h"
#include <stdio.h>		/* DEBUG PURPOSE */

#define MCU_BASE 8

uint8_t R(uint8_t y, uint8_t Cb, uint8_t Cr)
{
   int16_t res_16 = y - 0.0009267 * (Cb - 128) + 1.4016868 * (Cr - 128);
   uint8_t res_8;

   if (res_16 > 255)
      res_8 = 255;
   else if (res_16 < 0)
      res_8 = 0;
   else
      res_8 = res_16;

   return res_8;
}

uint8_t G(uint8_t y, uint8_t Cb, uint8_t Cr)
{
   int16_t res_16 = y - 0.3436954 * (Cb - 128) - 0.7141690 * (Cr - 128);
   uint8_t res_8;

   if (res_16 > 255)
      res_8 = 255;
   else if (res_16 < 0)
      res_8 = 0;
   else
      res_8 = res_16;

   return res_8;
}

uint8_t B(uint8_t y, uint8_t Cb, uint8_t Cr)
{
   int16_t res_16 = y + 1.7721604 * (Cb - 128) + 0.0009902 * (Cr - 128);
   uint8_t res_8;

   if (res_16 > 255)
      res_8 = 255;
   else if (res_16 < 0)
      res_8 = 0;
   else
      res_8 = res_16;

   return res_8;
}

void YCbCr_to_ARGB(uint8_t  *mcu_YCbCr[3], uint32_t *mcu_RGB,
		   uint32_t nb_blocks_h, uint32_t nb_blocks_v)
{
   /* R = Y − 0.0009267 × (Cb − 128) + 1.4016868 × (Cr − 128) */
   /* G = Y − 0.3436954 × (Cb − 128) − 0.7141690 × (Cr − 128) */
   /* B = Y + 1.7721604 × (Cb − 128) + 0.0009902 × (Cr − 128) */

   for (uint32_t i = 0;
	i < MCU_BASE * MCU_BASE * nb_blocks_v * nb_blocks_h;
	i++) {
      printf ("%d ", i);
      mcu_RGB[i]  = 0;	/* Mise à 0, nécessaire pour A */
      mcu_RGB[i] |= R (mcu_YCbCr[0][i], mcu_YCbCr[1][i], mcu_YCbCr[2][i]) << 16;
      mcu_RGB[i] |= G (mcu_YCbCr[0][i], mcu_YCbCr[1][i], mcu_YCbCr[2][i]) << 8;
      mcu_RGB[i] |= B (mcu_YCbCr[0][i], mcu_YCbCr[1][i], mcu_YCbCr[2][i]);
   }

   printf ("\n");
}
