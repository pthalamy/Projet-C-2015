#ifndef __BITSTREAM_ENC_H__
#define __BITSTREAM_ENC_H__

#include <stdint.h>
#include <stdbool.h>

struct bitstream;

extern struct bitstream *create_bitstream(const char *filename);

extern uint8_t write_bitstream(struct bitstream *stream,
			       uint8_t nb_bits, uint32_t *src);

extern void free_bitstream(struct bitstream *stream);


#endif
