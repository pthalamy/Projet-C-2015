#ifndef __PACK_H__
#define __PACK_H__

#include "bitstream.h"
#include "huffman.h"
#include <stdint.h>

/*retourne la magnitude de val*/
extern uint8_t magnitude (int32_t val);


/*Codage différentiel du coeff DC*/
extern void diff_DC(struct bitstream *stream, int32_t  *pred_DC,
		    struct abr *table_DC, int32_t bloc[64] );


/*Codage RLE des coeffs AC*/
extern void RLE_AC(struct bitstream *stream,
		   int32_t bloc[64],
		   struct abr *table_AC
   );

#endif
