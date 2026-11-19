/*
 * bmp.h - the BMP frames of a GIF (pure functions, host tested).
 *
 * Accepts what the GIF-to-BMP converters produce: uncompressed 24-bit, and
 * 32-bit uncompressed or BI_BITFIELDS with byte-aligned masks. Any size up to
 * BMP_MAX_WIDTH; bottom-up or top-down rows; rows padded to 4 bytes.
 */
#ifndef APP_BMP_H
#define APP_BMP_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BMP_HEADER_MIN  54u        /* file header + BITMAPINFOHEADER */
#define BMP_HEADER_READ 70u        /* enough for the bitfield masks too */
#define BMP_MAX_WIDTH   128

typedef struct {
  uint32_t data_offset;   /* file offset of the pixel data */
  int32_t  width;
  int32_t  height;        /* always positive */
  int      top_down;      /* 1: first stored row is the top of the picture */
  unsigned bpp;           /* 24 or 32 */
  uint32_t stride;        /* bytes per stored row, padding included */
  uint8_t  shift_r, shift_g, shift_b;   /* bit position of each channel (32-bit) */
} BmpInfo;

/* Parses the first len bytes of a file. Returns 0, or a negative reason:
 * -1 too short / not "BM", -2 unsupported format, -3 bad size. */
int Bmp_Parse(const uint8_t *hdr, size_t len, BmpInfo *out);
const char *Bmp_Error(int code);

/* File offset of the stored row that holds picture row y (0 = top) */
uint32_t Bmp_RowOffset(const BmpInfo *bi, int32_t y);

/* Pixel x of one stored row */
void Bmp_Pixel(const BmpInfo *bi, const uint8_t *row, int32_t x, uint8_t *r, uint8_t *g, uint8_t *b);

/* Nearest-neighbour source coordinate for destination d of dst_n (0..src_n-1) */
int32_t Bmp_SourceIndex(int32_t d, int32_t dst_n, int32_t src_n);

#ifdef __cplusplus
}
#endif

#endif
