#ifndef __IQZZ_H__
#define __IQZZ_H__

#include <stdint.h>

enum type_quantif {
   L = 0,
   C = 1
};

extern uint8_t table_quantif[2][64];

extern void iqzz_enc(int32_t in[64], int32_t out[64], enum type_quantif q);

#endif
