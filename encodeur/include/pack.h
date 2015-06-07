#ifndef __PACK_H__
#define __PACK_H__

#include "bitstream.h"

#include <stdint.h>

extern void diff_DC(struct bitstream *stream, int32_t  *pred_DC
		    /* ,struct huff_table *table_DC*/, int32_t bloc[64] );

#endif
