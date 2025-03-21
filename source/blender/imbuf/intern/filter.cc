/* SPDX-FileCopyrightText: 2001-2002 NaN Holding BV. All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

/** \file
 * \ingroup imbuf
 */

#include <cmath>

#include "MEM_guardedalloc.h"

#include "BLI_math_base.h"
#include "BLI_utildefines.h"

#include "IMB_filter.hh"
#include "IMB_imbuf.hh"
#include "IMB_imbuf_types.hh"

static void filtcolum(uchar *point, int y, int skip)
{
  uint c1, c2, c3, error;
  uchar *point2;

  if (y > 1) {
    c1 = c2 = *point;
    point2 = point;
    error = 2;
    for (y--; y > 0; y--) {
      point2 += skip;
      c3 = *point2;
      c1 += (c2 << 1) + c3 + error;
      error = c1 & 3;
      *point = c1 >> 2;
      point = point2;
      c1 = c2;
      c2 = c3;
    }
    *point = (c1 + (c2 << 1) + c2 + error) >> 2;
  }
}

static void filtcolumf(float *point, int y, int skip)
{
  float c1, c2, c3, *point2;

  if (y > 1) {
    c1 = c2 = *point;
    point2 = point;
    for (y--; y > 0; y--) {
      point2 += skip;
      c3 = *point2;
      c1 += (c2 * 2) + c3;
      *point = 0.25f * c1;
      point = point2;
      c1 = c2;
      c2 = c3;
    }
    *point = 0.25f * (c1 + (c2 * 2) + c2);
  }
}

void IMB_filtery(ImBuf *ibuf)
{
  uchar *point = ibuf->byte_buffer.data;
  float *pointf = ibuf->float_buffer.data;

  int x = ibuf->x;
  int y = ibuf->y;
  int skip = x << 2;

  for (; x > 0; x--) {
    if (point) {
      if (ibuf->planes > 24) {
        filtcolum(point, y, skip);
      }
      point++;
      filtcolum(point, y, skip);
      point++;
      filtcolum(point, y, skip);
      point++;
      filtcolum(point, y, skip);
      point++;
    }
    if (pointf) {
      if (ibuf->planes > 24) {
        filtcolumf(pointf, y, skip);
      }
      pointf++;
      filtcolumf(pointf, y, skip);
      pointf++;
      filtcolumf(pointf, y, skip);
      pointf++;
      filtcolumf(pointf, y, skip);
      pointf++;
    }
  }
}

static void imb_filterN(ImBuf *out, ImBuf *in)
{
  BLI_assert(out->channels == in->channels);
  BLI_assert(out->x == in->x && out->y == in->y);

  const int channels = in->channels;
  const int rowlen = in->x;

  if (in->byte_buffer.data && out->byte_buffer.data) {
    for (int y = 0; y < in->y; y++) {
      /* setup rows */
      const char *row2 = (const char *)in->byte_buffer.data + y * channels * rowlen;
      const char *row1 = (y == 0) ? row2 : row2 - channels * rowlen;
      const char *row3 = (y == in->y - 1) ? row2 : row2 + channels * rowlen;

      char *cp = (char *)out->byte_buffer.data + y * channels * rowlen;

      for (int x = 0; x < rowlen; x++) {
        const char *r11, *r13, *r21, *r23, *r31, *r33;

        if (x == 0) {
          r11 = row1;
          r21 = row2;
          r31 = row3;
        }
        else {
          r11 = row1 - channels;
          r21 = row2 - channels;
          r31 = row3 - channels;
        }

        if (x == rowlen - 1) {
          r13 = row1;
          r23 = row2;
          r33 = row3;
        }
        else {
          r13 = row1 + channels;
          r23 = row2 + channels;
          r33 = row3 + channels;
        }

        cp[0] = (r11[0] + 2 * row1[0] + r13[0] + 2 * r21[0] + 4 * row2[0] + 2 * r23[0] + r31[0] +
                 2 * row3[0] + r33[0]) >>
                4;
        cp[1] = (r11[1] + 2 * row1[1] + r13[1] + 2 * r21[1] + 4 * row2[1] + 2 * r23[1] + r31[1] +
                 2 * row3[1] + r33[1]) >>
                4;
        cp[2] = (r11[2] + 2 * row1[2] + r13[2] + 2 * r21[2] + 4 * row2[2] + 2 * r23[2] + r31[2] +
                 2 * row3[2] + r33[2]) >>
                4;
        cp[3] = (r11[3] + 2 * row1[3] + r13[3] + 2 * r21[3] + 4 * row2[3] + 2 * r23[3] + r31[3] +
                 2 * row3[3] + r33[3]) >>
                4;
        cp += channels;
        row1 += channels;
        row2 += channels;
        row3 += channels;
      }
    }
  }

  if (in->float_buffer.data && out->float_buffer.data) {
    for (int y = 0; y < in->y; y++) {
      /* setup rows */
      const float *row2 = (const float *)in->float_buffer.data + y * channels * rowlen;
      const float *row1 = (y == 0) ? row2 : row2 - channels * rowlen;
      const float *row3 = (y == in->y - 1) ? row2 : row2 + channels * rowlen;

      float *cp = out->float_buffer.data + y * channels * rowlen;

      for (int x = 0; x < rowlen; x++) {
        const float *r11, *r13, *r21, *r23, *r31, *r33;

        if (x == 0) {
          r11 = row1;
          r21 = row2;
          r31 = row3;
        }
        else {
          r11 = row1 - channels;
          r21 = row2 - channels;
          r31 = row3 - channels;
        }

        if (x == rowlen - 1) {
          r13 = row1;
          r23 = row2;
          r33 = row3;
        }
        else {
          r13 = row1 + channels;
          r23 = row2 + channels;
          r33 = row3 + channels;
        }

        cp[0] = (r11[0] + 2 * row1[0] + r13[0] + 2 * r21[0] + 4 * row2[0] + 2 * r23[0] + r31[0] +
                 2 * row3[0] + r33[0]) *
                (1.0f / 16.0f);
        cp[1] = (r11[1] + 2 * row1[1] + r13[1] + 2 * r21[1] + 4 * row2[1] + 2 * r23[1] + r31[1] +
                 2 * row3[1] + r33[1]) *
                (1.0f / 16.0f);
        cp[2] = (r11[2] + 2 * row1[2] + r13[2] + 2 * r21[2] + 4 * row2[2] + 2 * r23[2] + r31[2] +
                 2 * row3[2] + r33[2]) *
                (1.0f / 16.0f);
        cp[3] = (r11[3] + 2 * row1[3] + r13[3] + 2 * r21[3] + 4 * row2[3] + 2 * r23[3] + r31[3] +
                 2 * row3[3] + r33[3]) *
                (1.0f / 16.0f);
        cp += channels;
        row1 += channels;
        row2 += channels;
        row3 += channels;
      }
    }
  }
}

void IMB_mask_filter_extend(char *mask, int width, int height)
{
  const char *row1, *row2, *row3;
  int rowlen, x, y;
  char *temprect;

  rowlen = width;

  /* make a copy, to prevent flooding */
  temprect = static_cast<char *>(MEM_dupallocN(mask));

  for (y = 1; y <= height; y++) {
    /* setup rows */
    row1 = (char *)(temprect + (y - 2) * rowlen);
    row2 = row1 + rowlen;
    row3 = row2 + rowlen;
    if (y == 1) {
      row1 = row2;
    }
    else if (y == height) {
      row3 = row2;
    }

    for (x = 0; x < rowlen; x++) {
      if (mask[((y - 1) * rowlen) + x] == 0) {
        if (*row1 || *row2 || *row3 || *(row1 + 1) || *(row3 + 1)) {
          mask[((y - 1) * rowlen) + x] = FILTER_MASK_MARGIN;
        }
        else if ((x != rowlen - 1) && (*(row1 + 2) || *(row2 + 2) || *(row3 + 2))) {
          mask[((y - 1) * rowlen) + x] = FILTER_MASK_MARGIN;
        }
      }

      if (x != 0) {
        row1++;
        row2++;
        row3++;
      }
    }
  }

  MEM_freeN(temprect);
}

void IMB_mask_clear(ImBuf *ibuf, const char *mask, int val)
{
  int x, y;
  if (ibuf->float_buffer.data) {
    for (x = 0; x < ibuf->x; x++) {
      for (y = 0; y < ibuf->y; y++) {
        if (mask[ibuf->x * y + x] == val) {
          float *col = ibuf->float_buffer.data + 4 * (ibuf->x * y + x);
          col[0] = col[1] = col[2] = col[3] = 0.0f;
        }
      }
    }
  }
  else {
    /* char buffer */
    for (x = 0; x < ibuf->x; x++) {
      for (y = 0; y < ibuf->y; y++) {
        if (mask[ibuf->x * y + x] == val) {
          char *col = (char *)(ibuf->byte_buffer.data + 4 * ibuf->x * y + x);
          col[0] = col[1] = col[2] = col[3] = 0;
        }
      }
    }
  }
}

static int filter_make_index(const int x, const int y, const int w, const int h)
{
  if (x < 0 || x >= w || y < 0 || y >= h) {
    return -1; /* return bad index */
  }

  return y * w + x;
}

static int check_pixel_assigned(
    const void *buffer, const char *mask, const int index, const int depth, const bool is_float)
{
  int res = 0;

  if (index >= 0) {
    const int alpha_index = depth * index + (depth - 1);

    if (mask != nullptr) {
      res = mask[index] != 0 ? 1 : 0;
    }
    else if ((is_float && ((const float *)buffer)[alpha_index] != 0.0f) ||
             (!is_float && ((const uchar *)buffer)[alpha_index] != 0))
    {
      res = 1;
    }
  }

  return res;
}

void IMB_filter_extend(ImBuf *ibuf, char *mask, int filter)
{
  const int width = ibuf->x;
  const int height = ibuf->y;
  const int depth = 4; /* always 4 channels */
  const int chsize = ibuf->float_buffer.data ? sizeof(float) : sizeof(uchar);
  const size_t bsize = size_t(width) * height * depth * chsize;
  const bool is_float = (ibuf->float_buffer.data != nullptr);
  void *dstbuf = MEM_dupallocN(ibuf->float_buffer.data ? (void *)ibuf->float_buffer.data :
                                                         (void *)ibuf->byte_buffer.data);
  char *dstmask = mask == nullptr ? nullptr : (char *)MEM_dupallocN(mask);
  void *srcbuf = ibuf->float_buffer.data ? (void *)ibuf->float_buffer.data :
                                           (void *)ibuf->byte_buffer.data;
  char *srcmask = mask;
  int cannot_early_out = 1, r, n, k, i, j, c;
  float weight[25];

  /* build a weights buffer */
  n = 1;

#if 0
  k = 0;
  for (i = -n; i <= n; i++) {
    for (j = -n; j <= n; j++) {
      weight[k++] = sqrt(float(i) * i + j * j);
    }
  }
#endif

  weight[0] = 1;
  weight[1] = 2;
  weight[2] = 1;
  weight[3] = 2;
  weight[4] = 0;
  weight[5] = 2;
  weight[6] = 1;
  weight[7] = 2;
  weight[8] = 1;

  /* run passes */
  for (r = 0; cannot_early_out == 1 && r < filter; r++) {
    int x, y;
    cannot_early_out = 0;

    for (y = 0; y < height; y++) {
      for (x = 0; x < width; x++) {
        const int index = filter_make_index(x, y, width, height);

        /* only update unassigned pixels */
        if (!check_pixel_assigned(srcbuf, srcmask, index, depth, is_float)) {
          float tmp[4];
          float wsum = 0;
          float acc[4] = {0, 0, 0, 0};
          k = 0;

          if (check_pixel_assigned(
                  srcbuf, srcmask, filter_make_index(x - 1, y, width, height), depth, is_float) ||
              check_pixel_assigned(
                  srcbuf, srcmask, filter_make_index(x + 1, y, width, height), depth, is_float) ||
              check_pixel_assigned(
                  srcbuf, srcmask, filter_make_index(x, y - 1, width, height), depth, is_float) ||
              check_pixel_assigned(
                  srcbuf, srcmask, filter_make_index(x, y + 1, width, height), depth, is_float))
          {
            for (i = -n; i <= n; i++) {
              for (j = -n; j <= n; j++) {
                if (i != 0 || j != 0) {
                  const int tmpindex = filter_make_index(x + i, y + j, width, height);

                  if (check_pixel_assigned(srcbuf, srcmask, tmpindex, depth, is_float)) {
                    if (is_float) {
                      for (c = 0; c < depth; c++) {
                        tmp[c] = ((const float *)srcbuf)[depth * tmpindex + c];
                      }
                    }
                    else {
                      for (c = 0; c < depth; c++) {
                        tmp[c] = float(((const uchar *)srcbuf)[depth * tmpindex + c]);
                      }
                    }

                    wsum += weight[k];

                    for (c = 0; c < depth; c++) {
                      acc[c] += weight[k] * tmp[c];
                    }
                  }
                }
                k++;
              }
            }

            if (wsum != 0) {
              for (c = 0; c < depth; c++) {
                acc[c] /= wsum;
              }

              if (is_float) {
                for (c = 0; c < depth; c++) {
                  ((float *)dstbuf)[depth * index + c] = acc[c];
                }
              }
              else {
                for (c = 0; c < depth; c++) {
                  ((uchar *)dstbuf)[depth * index + c] = acc[c] > 255 ?
                                                             255 :
                                                             (acc[c] < 0 ? 0 :
                                                                           uchar(roundf(acc[c])));
                }
              }

              if (dstmask != nullptr) {
                dstmask[index] = FILTER_MASK_MARGIN; /* assigned */
              }
              cannot_early_out = 1;
            }
          }
        }
      }
    }

    /* keep the original buffer up to date. */
    memcpy(srcbuf, dstbuf, bsize);
    if (dstmask != nullptr) {
      memcpy(srcmask, dstmask, size_t(width) * height);
    }
  }

  /* free memory */
  MEM_freeN(dstbuf);
  if (dstmask != nullptr) {
    MEM_freeN(dstmask);
  }
}

void IMB_remakemipmap(ImBuf *ibuf, int use_filter)
{
  ImBuf *hbuf = ibuf;
  int curmap = 0;

  ibuf->miptot = 1;

  while (curmap < IMB_MIPMAP_LEVELS) {

    if (ibuf->mipmap[curmap]) {

      if (use_filter) {
        ImBuf *nbuf = IMB_allocImBuf(hbuf->x, hbuf->y, hbuf->planes, hbuf->flags);
        imb_filterN(nbuf, hbuf);
        imb_onehalf_no_alloc(ibuf->mipmap[curmap], nbuf);
        IMB_freeImBuf(nbuf);
      }
      else {
        imb_onehalf_no_alloc(ibuf->mipmap[curmap], hbuf);
      }
    }

    ibuf->miptot = curmap + 2;
    hbuf = ibuf->mipmap[curmap];
    if (hbuf) {
      hbuf->miplevel = curmap + 1;
    }

    if (!hbuf || (hbuf->x <= 2 && hbuf->y <= 2)) {
      break;
    }

    curmap++;
  }
}

void IMB_makemipmap(ImBuf *ibuf, int use_filter)
{
  ImBuf *hbuf = ibuf;
  int curmap = 0;

  IMB_free_mipmaps(ibuf);

  /* no mipmap for non RGBA images */
  if (ibuf->float_buffer.data && ibuf->channels < 4) {
    return;
  }

  ibuf->miptot = 1;

  while (curmap < IMB_MIPMAP_LEVELS) {
    if (use_filter) {
      ImBuf *nbuf = IMB_allocImBuf(hbuf->x, hbuf->y, hbuf->planes, hbuf->flags);
      imb_filterN(nbuf, hbuf);
      ibuf->mipmap[curmap] = IMB_onehalf(nbuf);
      IMB_freeImBuf(nbuf);
    }
    else {
      ibuf->mipmap[curmap] = IMB_onehalf(hbuf);
    }

    ibuf->miptot = curmap + 2;
    hbuf = ibuf->mipmap[curmap];
    hbuf->miplevel = curmap + 1;

    if (hbuf->x < 2 && hbuf->y < 2) {
      break;
    }

    curmap++;
  }
}

ImBuf *IMB_getmipmap(ImBuf *ibuf, int level)
{
  CLAMP(level, 0, ibuf->miptot - 1);
  return (level == 0) ? ibuf : ibuf->mipmap[level - 1];
}

void IMB_premultiply_rect(uint8_t *rect, char planes, int w, int h)
{
  uint8_t *cp;
  int x, y, val;

  if (planes == 24) { /* put alpha at 255 */
    cp = rect;

    for (y = 0; y < h; y++) {
      for (x = 0; x < w; x++, cp += 4) {
        cp[3] = 255;
      }
    }
  }
  else {
    cp = rect;

    for (y = 0; y < h; y++) {
      for (x = 0; x < w; x++, cp += 4) {
        val = cp[3];
        cp[0] = (cp[0] * val) >> 8;
        cp[1] = (cp[1] * val) >> 8;
        cp[2] = (cp[2] * val) >> 8;
      }
    }
  }
}

void IMB_premultiply_rect_float(float *rect_float, int channels, int w, int h)
{
  float val, *cp;
  int x, y;

  if (channels == 4) {
    cp = rect_float;
    for (y = 0; y < h; y++) {
      for (x = 0; x < w; x++, cp += 4) {
        val = cp[3];
        cp[0] = cp[0] * val;
        cp[1] = cp[1] * val;
        cp[2] = cp[2] * val;
      }
    }
  }
}

void IMB_premultiply_alpha(ImBuf *ibuf)
{
  if (ibuf == nullptr) {
    return;
  }

  if (ibuf->byte_buffer.data) {
    IMB_premultiply_rect(ibuf->byte_buffer.data, ibuf->planes, ibuf->x, ibuf->y);
  }

  if (ibuf->float_buffer.data) {
    IMB_premultiply_rect_float(ibuf->float_buffer.data, ibuf->channels, ibuf->x, ibuf->y);
  }
}

void IMB_unpremultiply_rect(uint8_t *rect, char planes, int w, int h)
{
  uchar *cp;
  int x, y;
  float val;

  if (planes == 24) { /* put alpha at 255 */
    cp = rect;

    for (y = 0; y < h; y++) {
      for (x = 0; x < w; x++, cp += 4) {
        cp[3] = 255;
      }
    }
  }
  else {
    cp = rect;

    for (y = 0; y < h; y++) {
      for (x = 0; x < w; x++, cp += 4) {
        val = cp[3] != 0 ? 1.0f / float(cp[3]) : 1.0f;
        cp[0] = unit_float_to_uchar_clamp(cp[0] * val);
        cp[1] = unit_float_to_uchar_clamp(cp[1] * val);
        cp[2] = unit_float_to_uchar_clamp(cp[2] * val);
      }
    }
  }
}

void IMB_unpremultiply_rect_float(float *rect_float, int channels, int w, int h)
{
  float val, *fp;
  int x, y;

  if (channels == 4) {
    fp = rect_float;
    for (y = 0; y < h; y++) {
      for (x = 0; x < w; x++, fp += 4) {
        val = fp[3] != 0.0f ? 1.0f / fp[3] : 1.0f;
        fp[0] = fp[0] * val;
        fp[1] = fp[1] * val;
        fp[2] = fp[2] * val;
      }
    }
  }
}

void IMB_unpremultiply_alpha(ImBuf *ibuf)
{
  if (ibuf == nullptr) {
    return;
  }

  if (ibuf->byte_buffer.data) {
    IMB_unpremultiply_rect(ibuf->byte_buffer.data, ibuf->planes, ibuf->x, ibuf->y);
  }

  if (ibuf->float_buffer.data) {
    IMB_unpremultiply_rect_float(ibuf->float_buffer.data, ibuf->channels, ibuf->x, ibuf->y);
  }
}
