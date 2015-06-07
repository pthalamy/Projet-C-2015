#ifndef __TIFF_H__
#define __TIFF_H__

struct tiff_file_desc;

extern struct tiff_file_desc *create_tfd_and_read_header (const char *file_name);

extern void read_TIFF_ifd(struct tiff_file_desc *tfd);

extern void free_tfd (struct tiff_file_desc *tfd);

#endif
