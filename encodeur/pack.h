#include <stdint.h>
#include "bitstream_enc.h"

extern void diff_DC(struct bitstream *stream,
	     int32_t  *pred_DC,
	     //struct huff_table *table_DC,
		    int32_t bloc[64] );
