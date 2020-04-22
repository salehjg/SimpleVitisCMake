#define N 128
#define DATA_SIZE 1 << 10

// Tripcount identifiers
const int c_len = DATA_SIZE;
const int c_n = N;

// This kernel is accessing 2 global variables and storing the result into
// a third global variable. This type of access does not yield good
// performance because this kernel has only one global memory port.

extern "C" {
void vadd(int *c, const int *a, const int *b, const int len) {
#pragma HLS INTERFACE m_axi port = c offset = slave bundle = gmem
#pragma HLS INTERFACE m_axi port = a offset = slave bundle = gmem
#pragma HLS INTERFACE m_axi port = b offset = slave bundle = gmem
#pragma HLS INTERFACE s_axilite port = c bundle = control
#pragma HLS INTERFACE s_axilite port = a bundle = control
#pragma HLS INTERFACE s_axilite port = b bundle = control
#pragma HLS INTERFACE s_axilite port = len bundle = control
#pragma HLS INTERFACE s_axilite port = return bundle = control

vadd_loop:
    for (int x = 0; x < len; ++x) {
       #pragma HLS LOOP_TRIPCOUNT min=c_len max=c_len
        c[x] = a[x] + b[x];
    }
}
}
