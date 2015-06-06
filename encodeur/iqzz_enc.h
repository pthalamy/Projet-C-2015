#include <stdint.h>

enum type_quantif{
   L,
   C
};



void iqzz_enc(int32_t in[64], int32_t out[64], enum type_quantif q);
