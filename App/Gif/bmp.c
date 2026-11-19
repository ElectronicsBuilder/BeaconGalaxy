/*
 * bmp.c - see bmp.h.
 */
#include "bmp.h"

#define BI_RGB        0u
#define BI_BITFIELDS  3u

static uint32_t P_U32(const uint8_t *p) { return p[0] | (p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24); }
static uint16_t P_U16(const uint8_t *p) { return (uint16_t)(p[0] | (p[1] << 8)); }

/* Byte-aligned 8-bit mask -> shift; 255 if not usable */
static uint8_t P_MaskShift(uint32_t mask)
{
  for (uint8_t s = 0; s <= 24u; s += 8u)
    if (mask == (0xFFu << s)) return s;
  return 255u;
}

int Bmp_Parse(const uint8_t *h, size_t len, BmpInfo *bi)
{
  if (h == NULL || bi == NULL || len < BMP_HEADER_MIN || h[0] != 'B' || h[1] != 'M') return -1;
  uint32_t header_size = P_U32(h + 14);
  if (header_size < 40u) return -2;
  int32_t w = (int32_t)P_U32(h + 18);
  int32_t ht = (int32_t)P_U32(h + 22);
  uint16_t planes = P_U16(h + 26);
  unsigned bpp = P_U16(h + 28);
  uint32_t comp = P_U32(h + 30);

  bi->data_offset = P_U32(h + 10);
  bi->bpp = bpp;
  bi->top_down = ht < 0;
  bi->height = ht < 0 ? -ht : ht;
  bi->width = w;
  bi->shift_r = 16; bi->shift_g = 8; bi->shift_b = 0;

  if (planes != 1u || (bpp != 24u && bpp != 32u)) return -2;
  if (comp == BI_BITFIELDS)
  {
    if (bpp != 32u) return -2;
    /* masks follow a 40-byte header, or sit inside a V4/V5 header */
    if (len < 14u + 40u + 12u) return -1;
    uint8_t sr = P_MaskShift(P_U32(h + 54)), sg = P_MaskShift(P_U32(h + 58)), sb = P_MaskShift(P_U32(h + 62));
    if (sr == 255u || sg == 255u || sb == 255u) return -2;
    bi->shift_r = sr; bi->shift_g = sg; bi->shift_b = sb;
  }
  else if (comp != BI_RGB)
  {
    return -2;
  }
  if (w <= 0 || w > BMP_MAX_WIDTH || bi->height <= 0 || bi->height > 4096) return -3;
  bi->stride = ((uint32_t)w * (bpp / 8u) + 3u) & ~3u;
  if (bi->data_offset < 14u + header_size) return -3;
  return 0;
}

const char *Bmp_Error(int code)
{
  switch (code)
  {
    case 0:  return "ok";
    case -1: return "not a BMP file";
    case -2: return "unsupported BMP format (24/32-bit only)";
    case -3: return "bad BMP size";
    default: return "BMP error";
  }
}

uint32_t Bmp_RowOffset(const BmpInfo *bi, int32_t y)
{
  int32_t stored = bi->top_down ? y : bi->height - 1 - y;
  return bi->data_offset + (uint32_t)stored * bi->stride;
}

void Bmp_Pixel(const BmpInfo *bi, const uint8_t *row, int32_t x, uint8_t *r, uint8_t *g, uint8_t *b)
{
  if (bi->bpp == 24u)
  {
    const uint8_t *p = row + 3 * x;
    *b = p[0]; *g = p[1]; *r = p[2];
  }
  else
  {
    uint32_t v = P_U32(row + 4 * x);
    *r = (uint8_t)(v >> bi->shift_r);
    *g = (uint8_t)(v >> bi->shift_g);
    *b = (uint8_t)(v >> bi->shift_b);
  }
}

int32_t Bmp_SourceIndex(int32_t d, int32_t dst_n, int32_t src_n)
{
  if (dst_n <= 0 || src_n <= 0) return 0;
  if (src_n == dst_n) return d;
  int32_t s = (int32_t)(((int64_t)(2 * d + 1) * src_n) / (2 * dst_n));   /* sample the cell centre */
  return s < 0 ? 0 : (s >= src_n ? src_n - 1 : s);
}
