#include <stdio.h>
#include <stdlib.h>
#include <jpeglib.h>
#include <stdbool.h>
#include <math.h>

struct color {
    unsigned char r;
    unsigned char g;
    unsigned char b;
};

void setPixelColor(unsigned char * pixel, int r, int g, int b) {
    pixel[0] = r;
    pixel[1] = g;
    pixel[2] = b;
}

bool colorsContainsColor(struct color * colors, unsigned char * color, int colorsLength) {
    for(int i = 0; i < colorsLength; i++) {
        if((colors[i].r == color[0]) 
        && (colors[i].g == color[1]) 
        && (colors[i].b == color[2])) {
            return true;
        }
    }
    return false;
}

struct color setColorsElement(struct color currColor, unsigned char * color) {
    // printf("%d, %d, %d\n", color[0], color[1], color[2]);
    currColor.r = color[0];
    currColor.g = color[1];
    currColor.b = color[2];

    return currColor;
}

int findColorIndex(struct color * colors, unsigned char * color, int colorsLength) {
    for(int i = 0; i < colorsLength; i++) {
        if((colors[i].r == color[0]) 
        && (colors[i].g == color[1]) 
        && (colors[i].b == color[2])) {
            return i;
        }
    }
    return -1;
}

int main(int argc, char ** argv) {

    printf("%s", argv[1]);

    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;

    FILE* infile;
    FILE* outfile;
    JSAMPARRAY buffer;
    int row_stride;

    if((infile = fopen(argv[1], "rb")) == NULL) {
        fprintf(stderr, "can't open %s\n", argv[1]);
        return 1;
    }
    if ((outfile = fopen(argv[2], "wb")) == NULL) {
        fprintf(stderr, "can't open %s\n", argv[2]);
        return 1;
    }

    //Decompress input image
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);

    jpeg_stdio_src(&cinfo, infile);
    (void) jpeg_read_header(&cinfo, TRUE);
    (void) jpeg_start_decompress(&cinfo);

    row_stride = cinfo.output_width * cinfo.output_components;

    buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

    // Create compression object
    struct jpeg_compress_struct output_cinfo;
    struct jpeg_error_mgr output_jerr;

    output_cinfo.err = jpeg_std_error(&output_jerr);
    jpeg_create_compress(&output_cinfo);

    jpeg_stdio_dest(&output_cinfo, outfile);

    output_cinfo.image_width = cinfo.output_width;
    output_cinfo.image_height = cinfo.output_height;
    output_cinfo.input_components = cinfo.output_components;
    output_cinfo.in_color_space = cinfo.out_color_space;

    // printf("%d\n", cinfo.output_width);
    // printf("%d\n", cinfo.output_height);
    // printf("%d\n", cinfo.output_components);
    // printf("%d\n", cinfo.out_color_space);

    jpeg_set_defaults(&output_cinfo);

    jpeg_start_compress(&output_cinfo, TRUE);

    size_t array_size = 255 * 255 * 255;

    struct color *colorsArr = (struct color *)malloc(array_size * sizeof(struct color));
    int * colorsCount = (int *)malloc(array_size * sizeof(int));
    int j = 0;

    while(cinfo.output_scanline < cinfo.output_height) {
        (void) jpeg_read_scanlines(&cinfo, buffer, 1);
        // put_scanline_someplace(buffer[0], row_stride);

        for(int i = 0; i < row_stride; i+= cinfo.output_components) {
            printf("%d\n", j);
            int colorIndex;
            if(!colorsContainsColor(colorsArr, &(buffer[0][i]), j)) {
                colorsArr[j] = setColorsElement(colorsArr[j], &(buffer[0][i]));
                j++;
            }
            else if((colorIndex = findColorIndex(colorsArr, &(buffer[0][i]), j)) != -1) {
                colorsCount[j]++;
            }
        }

        for(int i = 0; i < row_stride; i += cinfo.output_components) {
            setPixelColor(&(buffer[0][i]), 0, 0, 255);
        }

        (void)jpeg_write_scanlines(&output_cinfo, buffer, 1);
    }

    jpeg_finish_compress(&output_cinfo);

    (void) jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);

    fclose(infile);

    return 0;
}