#ifndef DEFINE_H
#define DEFINE_H

#define ROWS_O          270 //行数
#define COLS_O          480 //列数
#define FILTER_SIZE     3   //滤波器大小，3×3
#define STRIDE          1
#define ROWS_I          (ROWS_O*STRIDE+FILTER_SIZE-1)
#define COLS_I          (COLS_O*STRIDE+FILTER_SIZE-1)

#define FPGA

#endif
