#define UNROLL 4

kernel void to_grayscale(
    read_only image2d_t rgb_img,
    write_only image2d_t luma_img
) {
    const int x = (int) get_global_id(0);
    const int y = (int) get_global_id(1);

    #pragma unroll 2
    for(uchar i = 0; i < UNROLL; i++) {
        uint4 rgb_pixel = read_imageui(
            rgb_img,
            (int2) ((UNROLL * x) + i, y)
        );

        uint gray = (
            rgb_pixel.r * 77 +
            rgb_pixel.g * 150 +
            rgb_pixel.b * 29
        ) >> 8;

        write_imageui(
            luma_img,
            (int2) ((UNROLL * x) + i, y),
            (uint4) (gray, gray, gray, 255)
        );
    }
}