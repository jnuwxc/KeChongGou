#include "string.h"
#include "2dconv.hpp"
#include "buffer.hpp"
#include "define.hpp"


void _2DConv(int8_t *in, int8_t *out, int8_t *filter, uint32_t nrows, uint32_t ncols, uint8_t k, uint8_t stride){
	uint16_t r, c, i, j;
	uint32_t nrows_i = nrows*stride+k-1;
	uint32_t ncols_i = ncols*stride+k-1;

	for(r=0; r<nrows; r++){
		for(c=0; c<ncols; c++){
#pragma HLS PIPELINE II=1
			for(i=0; i<k; i++){
				for(j=0; j<k; j++){
					uint32_t offset = (r*stride+i)*ncols_i+(c*stride+j);
					int32_t in_tmp = *(in+offset);
					int32_t filter_tmp = *(filter+i*k+j);
					*(out+r*ncols+c) += in_tmp*filter_tmp;
				}
			}
		}
	}
}

void _2DConv_OP(int8_t *in, int8_t *out,  uint32_t nrows, uint32_t ncols, uint8_t k, uint8_t stride){
	uint16_t r, c, i, j;
	uint32_t nrows_i = nrows*stride+k-1;
	uint32_t ncols_i = ncols*stride+k-1;

	int8_t *filter_base = in + ROWS_I*COLS_I;

    int8_t filter_buffer[FILTER_SIZE][FILTER_SIZE];
#pragma HLS ARRAY_PARTITION variable=filter_buffer complete dim=0
    memcpy(&filter_buffer[0][0], filter_base, FILTER_SIZE*FILTER_SIZE*sizeof(int8_t));

	for(r=0; r<nrows; r++){
		for(c=0; c<ncols; c++){
#pragma HLS PIPELINE II=1
			int8_t store_data = 0;
			for(i=0; i<k; i++){
				for(j=0; j<k; j++){
					uint32_t offset = (r*stride+i)*ncols_i+(c*stride+j);
					int8_t val = *(in+offset);
					int8_t res = 0;

					if(filter_buffer[i][j]==0){
							res = 0;
					}else if(filter_buffer[i][j]==1){
							res = val;
					}else if(filter_buffer[i][j]==-1){
							res = -val;
					}
					store_data += res;


					*(out+r*ncols+c) = store_data;
				}
			}
		}
	}
}


void HW_2DConv_Mmap_1(int8_t *pixel_in, int8_t *pixel_out, int32_t addr_reserved)
{
#pragma HLS INTERFACE m_axi depth = 482*272+3*3 port = pixel_in offset = slave bundle = user_axi_in register
#pragma HLS INTERFACE m_axi depth = 480*270 port = pixel_out offset = slave bundle = user_axi_out register

#pragma HLS INTERFACE s_axilite port = pixel_in bundle = user_axi4lite_config  register
#pragma HLS INTERFACE s_axilite port = pixel_out bundle = user_axi4lite_config  register
#pragma HLS INTERFACE s_axilite port = addr_reserved offset = 0xFFF0 bundle = user_axi4lite_config  register
#pragma HLS INTERFACE s_axilite port = return bundle = user_axi4lite_config  register

	int8_t *filter_base = pixel_in + ROWS_I*COLS_I;

	_2DConv(pixel_in, pixel_out, filter_base, ROWS_O, COLS_O, FILTER_SIZE, STRIDE);
}



void HW_2DConv_Mmap_2(int8_t *pixel_in, int8_t *pixel_out, int32_t addr_reserved)
{
#pragma HLS INTERFACE m_axi depth = 482*272+3*3 port = pixel_in offset = slave bundle = user_axi_in register
#pragma HLS INTERFACE m_axi depth = 480*270 port = pixel_out offset = slave bundle = user_axi_out register

#pragma HLS INTERFACE s_axilite port = pixel_in bundle = user_axi4lite_config  register
#pragma HLS INTERFACE s_axilite port = pixel_out bundle = user_axi4lite_config  register
#pragma HLS INTERFACE s_axilite port = addr_reserved offset = 0xFFF0 bundle = user_axi4lite_config  register
#pragma HLS INTERFACE s_axilite port = return bundle = user_axi4lite_config  register

	_2DConv_OP(pixel_in, pixel_out,  ROWS_O, COLS_O, FILTER_SIZE, STRIDE);
}


void HW_2DConv_Mmap_3(int8_t *pixel_in, int8_t *pixel_out, int32_t addr_reserved){
#pragma HLS INTERFACE m_axi depth = 482*272+3*3 port = pixel_in offset = slave bundle = user_axi_in register
#pragma HLS INTERFACE m_axi depth = 480*270 port = pixel_out offset = slave bundle = user_axi_out register

#pragma HLS INTERFACE s_axilite port = pixel_in bundle = user_axi4lite_config  register
#pragma HLS INTERFACE s_axilite port = pixel_out bundle = user_axi4lite_config  register
#pragma HLS INTERFACE s_axilite port = addr_reserved offset = 0xFFF0 bundle = user_axi4lite_config  register
#pragma HLS INTERFACE s_axilite port = return bundle = user_axi4lite_config  register

	uint16_t ii;
	uint16_t r, c, i, j;

	int8_t *in_base = pixel_in;
	int8_t *filter_base = pixel_in + ROWS_I*COLS_I;
	int8_t *out_base = pixel_out;

    int8_t filter_buffer[FILTER_SIZE][FILTER_SIZE];
#pragma HLS ARRAY_PARTITION variable=filter_buffer complete dim=0
    memcpy(&filter_buffer[0][0], filter_base, FILTER_SIZE*FILTER_SIZE*sizeof(int8_t));

    ap_linebuffer<int8_t, FILTER_SIZE, COLS_I> line_buffer;

	ROW_LOOP: for(r=0; r<ROWS_I; r++){
		ap_window<int8_t,FILTER_SIZE,FILTER_SIZE> window_buffer;

		//fill the line buffer
		COL_LOOP: for(c=0; c<COLS_I; c++){
            int8_t store_data = 0;
			int8_t load_data = *(in_base+r*COLS_I+c);

			line_buffer.shift_up(c);
			line_buffer.insert_bottom(load_data, c);

			if(r>=2)
			{
				window_buffer.shift_right();
				window_buffer.insert(line_buffer.getval(2,c), 0, 2);
				window_buffer.insert(line_buffer.getval(1,c), 1, 2);
				window_buffer.insert(line_buffer.getval(0,c), 2, 2);
			}

			if(r>=2 && c>=2)
			{
				for(i=0; i<FILTER_SIZE; i++){
					for(j=0; j<FILTER_SIZE; j++){
						int8_t res = 0;
						int8_t val = window_buffer.getval(i,j);
						if(filter_buffer[i][j]==0){
							res = 0;
						}else if(filter_buffer[i][j]==1){
							res = val;
						}else if(filter_buffer[i][j]==-1){
							res = -val;
						}
						store_data += res;
					}
				}
				*(out_base+(r-2)*COLS_O+(c-2)) = store_data;
			}
		}
	}
}




