#include <stdio.h>
#include <ap_int.h>
#include "2dconv.hpp"
#include "define.hpp"


int main(int argc, char** argv){
	int i, j;

	FILE *fpin, *fpout;
	fpin = fopen("in.txt", "wb");
	fpout = fopen("out.txt", "wb");
	uint8_t stride = STRIDE;
	uint32_t nrows_o = ROWS_O;
	uint32_t ncols_o = COLS_O;
	uint32_t nrows_i = nrows_o*stride;
	uint32_t ncols_i = ncols_o*stride;
	uint32_t k = FILTER_SIZE;

	int8_t *in_data, *out_data;

	in_data = (int8_t*)malloc((nrows_i+k-1)*(ncols_i+k-1)*sizeof(int8_t)); //padding image’s edge
	out_data = (int8_t*)malloc(nrows_o*ncols_o*sizeof(int8_t));

	int8_t filter[FILTER_SIZE][FILTER_SIZE] = {{-1, -1, 1}, {-1, 0, 1}, {-1, -1, 1}};

	printf("Initialize input data......\n"); fflush(stdout);
	for(i=0; i<nrows_i+k-1; i++){
		for(j=0; j<ncols_i+k-1; j++){
			if(i<nrows_i && j<ncols_i) *(in_data+i*(ncols_i+k-1)+j) = j;
			else *(in_data+i*(ncols_i+k-1)+j) = 0;       //pixels out of boundary are filled with 0
			fprintf(fpin, "%2d ", *(in_data+i*(ncols_i+k-1)+j)); fflush(stdout);
		}
		fprintf(fpin, "\n");
	}
	printf("Completed!\n"); fflush(stdout);
	fclose(fpin);
	printf("Filter:\n");
	for(i=0; i<3; i++){
		for(j=0; j<3; j++){
			printf("%2d ", filter[i][j]);
		}
		printf("\n");
	}

	memset(out_data, 0, nrows_o*ncols_o*sizeof(int8_t));

	int8_t buffer[ROWS_I*COLS_I+FILTER_SIZE*FILTER_SIZE+ROWS_O*COLS_O];

	int8_t* in_data_ptr = buffer;
	int8_t* filter_ptr = buffer + ROWS_I*COLS_I;
	int8_t* out_data_ptr = filter_ptr + FILTER_SIZE*FILTER_SIZE;
	memcpy(in_data_ptr, in_data, ROWS_I*COLS_I*sizeof(int8_t));
	memcpy(filter_ptr, &filter[0][0], FILTER_SIZE*FILTER_SIZE*sizeof(int8_t));


#ifndef FPGA   //Run CPU codes
	printf("Run 2D convolution......"); fflush(stdout);
	_2DConv(in_data, out_data, &filter[0][0], nrows_o, ncols_o, k, stride);
	printf("Completed!\n"); fflush(stdout);
#else
	printf("[HLS] Run 2D convolution......"); fflush(stdout);
	HW_2DConv_Mmap_1(in_data_ptr, out_data_ptr, 0);
	printf("Completed!\n"); fflush(stdout);
	memcpy(out_data, out_data_ptr, ROWS_O*COLS_O*sizeof(int8_t));
#endif
	printf("Print results of 2D convolution:\n"); fflush(stdout);
	for(i=0; i<nrows_o; i++){
		for(j=0; j<ncols_o; j++){
			fprintf(fpout, "%3d ", *(out_data+i*ncols_o+j));
		}
		fprintf(fpout, "\n");
	}
	fclose(fpout);
	return 0;
}

