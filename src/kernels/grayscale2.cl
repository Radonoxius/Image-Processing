#define UNROLL 4

kernel void to_grayscale_buffer_vload(
    global const uchar* restrict rgb_buf,
    global uchar* restrict luma_buf,
    const int width
) {
    // x represents the starting pixel block for this work-item
    const int x = (int) get_global_id(0);
    const int y = (int) get_global_id(1);

    #pragma unroll 2
    for(uchar i = 0; i < UNROLL; i++) {
        // Calculate current pixel column
        int current_x = (UNROLL * x) + i;

        // Boundary check
        if (current_x < width) {
            // Linear index for the 1-byte-per-pixel grayscale output
            int luma_index = y * width + current_x;
            
            // vload3 takes an element offset, not a byte offset.
            // Pass it the pixel index, and it automatically calculates (index * 3) bytes.
            uchar3 rgb_pixel = vload3(luma_index, rgb_buf);

            // Fixed-point grayscale conversion using vector components
            uchar gray = (uchar)((
                rgb_pixel.x * 77 +
                rgb_pixel.y * 150 +
                rgb_pixel.z * 29
            ) >> 8);

            // Write scalar back to the 1-byte-per-pixel luma buffer
            luma_buf[luma_index] = gray;
        }
    }
}