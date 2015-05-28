#include "bitstream.h"

struct bitstream *create_bitstream(const char *filename)
{

}

bool end_of_bitstream(struct bitstream *stream)
{

}

uint8_t read_bitstream(struct bitstream *stream,
		       uint8_t nb_bits, uint32_t *dest,
		       bool byte_stuffing)
{

}

void skip_bitstream_until(struct bitstream *stream,
			  uint8_t byte)
{

}

void free_bitstream(struct bitstream *stream)
{

}
