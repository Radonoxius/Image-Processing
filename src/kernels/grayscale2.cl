#define UNROLL 4

kernel void to_grayscale_buffer_vload(
    global const uchar* restrict rgb_buf,
    global uchar* restrict luma_buf,

    const int width
) {
    // x represents the starting pixel block for this work-item
    const int x = (int) get_global_id(0);
    const int y = (int) get_global_id(1);

    vload16()
}