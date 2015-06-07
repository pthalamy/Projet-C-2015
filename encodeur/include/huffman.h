#ifndef __HUFFMAN_H__
#define __HUFFMAN_H__

#include "bitstream.h"

#include <stdint.h>
#include <stdbool.h>

struct huff_table;

/* extern struct huff_table create_huffman_table(...); */

extern void store_huffman_table(struct bitstream *stream, struct huff_table *ht);

extern void free_huffman_table(struct huff_table *table);

#endif
