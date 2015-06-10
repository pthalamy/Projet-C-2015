#ifndef __TIFF_H__
#define __TIFF_H__

#include <stdint.h>

struct tiff_file_desc;

extern struct tiff_file_desc *create_tfd_and_read_header (const char *file_name);

extern void read_TIFF_ifd(struct tiff_file_desc *tfd, uint32_t *height, uint32_t *width);

extern void get_tiff_scan_data (struct tiff_file_desc *tfd);

extern uint32_t **split_scan_into_16x16_MCU(struct tiff_file_desc *tfd, uint32_t *nbMCUH, uint32_t *nbMCUV);

extern void free_tfd (struct tiff_file_desc *tfd);

#endif
