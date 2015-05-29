#include "conv.h"

#define MCU_BASE 8

uint8_t R(uint8_t y, uint8_t Cb, uint8_t Cr)
{
   return y - 0.0009267 * (Cb - 128) + 1.4016868 * (Cr - 128);
}

uint8_t G(uint8_t y, uint8_t Cb, uint8_t Cr)
{
   return y - 0.3436954 * (Cb - 128) - 0.7141690 * (Cr - 128);
}

uint8_t B(uint8_t y, uint8_t Cb, uint8_t Cr)
{
   return y + 1.7721604 * (Cb - 128) + 0.0009902 * (Cr - 128);
}

void YCbCr_to_ARGB(uint8_t  *mcu_YCbCr[3], uint32_t *mcu_RGB,
		   uint32_t nb_blocks_h, uint32_t nb_blocks_v)
{
   /* R = Y − 0.0009267 × (Cb − 128) + 1.4016868 × (Cr − 128) */
   /* G = Y − 0.3436954 × (Cb − 128) − 0.7141690 × (Cr − 128) */
   /* B = Y + 1.7721604 × (Cb − 128) + 0.0009902 × (Cr − 128) */
   uint32_t k = 0;		/* Indice courant de la mcu RGB */

   for (uint32_t i = 0; i < MCU_BASE * nb_blocks_v; i++) {
      for (uint32_t j = 0; j < MCU_BASE * nb_blocks_h; j++) {
	 mcu_RGB[k] = 0;	/* Mise à 0, nécessaire pour A */
	 mcu_RGB[k] |= R (mcu_YCbCr[0], mcu_YCbCr[1], mcu_YCbCr[2]) << 16;
	 mcu_RGB[k] |= G (mcu_YCbCr[0], mcu_YCbCr[1], mcu_YCbCr[2]) << 8;
	 mcu_RGB[k] |= B (mcu_YCbCr[0], mcu_YCbCr[1], mcu_YCbCr[2]);
	 ++k;
      }
   }
}
