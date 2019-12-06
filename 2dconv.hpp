#ifndef _2DCONV_H
#define _2DCONV_H

#include <stdio.h>
#include <ap_int.h>
#include <stdint.h>
#include <hls_stream.h>

using namespace std;

void _2DConv(int8_t *in, int8_t *out, int8_t *filter, uint32_t nrows, uint32_t ncols, uint8_t k, uint8_t stride);

void _2DConv_OP(int8_t *in, int8_t *out,  uint32_t nrows, uint32_t ncols, uint8_t k, uint8_t stride);

void HW_2DConv_Mmap_1(int8_t *pixel_in, int8_t *pixel_out, int32_t addr_reserved);

void HW_2DConv_Mmap_2(int8_t *pixel_in, int8_t *pixel_out, int32_t addr_reserved);

void HW_2DConv_Mmap_3(int8_t *pixel_in, int8_t *pixel_out, int32_t addr_reserved);

#endif
