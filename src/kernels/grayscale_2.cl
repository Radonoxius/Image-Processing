kernel void to_grayscale(
    read_only image2d_t rgb_img,
    write_only image2d_t luma_img
) {
    const int x = (int) get_global_id(0);
    const int y = (int) get_global_id(1);

    uint4 rgb_pixel0 = read_imageui(
        rgb_img,
        (int2) (x, y)
    );
    //uint4 rgb_pixel1 = read_imageui(
    //    rgb_img,
    //    (int2) (2 * x + 1, y)
    //);

    uint gray0 = (
        rgb_pixel0.r * 77 +
        rgb_pixel0.g * 150 +
        rgb_pixel0.b * 29
    ) >> 8;
    //uint gray1 = (
    //    rgb_pixel1.r * 77 +
    //    rgb_pixel1.g * 150 +
    //    rgb_pixel1.b * 29
    //) >> 8;

    write_imageui(
        luma_img,
        (int2) (x, y),
        (uint4) (gray0, gray0, gray0, 255)
    );
    //write_imageui(
    //    luma_img,
    //    (int2) (2 * x + 1, y),
    //    (uint4) (gray1, gray1, gray1, 255)
    //);
}