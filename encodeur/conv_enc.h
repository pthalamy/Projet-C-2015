#include <stdint.h>

extern void RGB_to_YCbCr(uint8_t *mcu_YCbCr[3], uint32_t *mcu_RGB, uint32_t nb_blocks_h, uint32_t nb_blocks_v);