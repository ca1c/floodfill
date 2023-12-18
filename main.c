#include <stdio.h>
#include <stdlib.h>
#include <jpeglib.h>

#define MAX_COLORS 256

// store RGB values
typedef struct {
    int r, g, b;
} RGB;

// find the most common color in the image
RGB findMostCommonColor(JSAMPLE* image, int width, int height) {
    int colorCount[MAX_COLORS][3] = {0};

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int index = y * width * 3 + x * 3;
            colorCount[image[index]][0]++;
            colorCount[image[index + 1]][1]++;
            colorCount[image[index + 2]][2]++;
        }
    }

    int maxCount = 0;
    RGB mostCommonColor = {0, 0, 0};

    for (int i = 0; i < MAX_COLORS; i++) {
        if (colorCount[i][0] + colorCount[i][1] + colorCount[i][2] > maxCount) {
            maxCount = colorCount[i][0] + colorCount[i][1] + colorCount[i][2];
            mostCommonColor.r = i;
            mostCommonColor.g = i;
            mostCommonColor.b = i;
        }
    }

    return mostCommonColor;
}

// replace the most common color with a new color
void replaceColor(JSAMPLE* image, int width, int height, RGB oldColor, RGB newColor) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int index = y * width * 3 + x * 3;
            if (image[index] == oldColor.r && image[index + 1] == oldColor.g && image[index + 2] == oldColor.b) {
                image[index] = newColor.r;
                image[index + 1] = newColor.g;
                image[index + 2] = newColor.b;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <input.jpg> <output.jpg> <newColor>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *inputFileName = argv[1];
    const char *outputFileName = argv[2];
    RGB newColor;
    sscanf(argv[3], "%x", &newColor.r); // Assuming newColor is given in hexadecimal format

    FILE *infile = fopen(inputFileName, "rb");
    if (!infile) {
        fprintf(stderr, "Error opening input file\n");
        exit(EXIT_FAILURE);
    }

    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);

    jpeg_stdio_src(&cinfo, infile);
    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);

    int width = cinfo.output_width;
    int height = cinfo.output_height;
    int numChannels = cinfo.output_components;
    int rowStride = width * numChannels;
    JSAMPLE *image = (JSAMPLE *)malloc(rowStride * height);

    JSAMPROW row_pointer[1];
    while (cinfo.output_scanline < cinfo.output_height) {
        row_pointer[0] = &image[(cinfo.output_scanline) * rowStride];
        jpeg_read_scanlines(&cinfo, row_pointer, 1);
    }

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);

    // Find the most common color in the image
    RGB mostCommonColor = findMostCommonColor(image, width, height);

    // Replace the most common color with the new color
    replaceColor(image, width, height, mostCommonColor, newColor);

    // Write the modified image to a new file
    FILE *outfile = fopen(outputFileName, "wb");
    if (!outfile) {
        fprintf(stderr, "Error opening output file\n");
        free(image);
        exit(EXIT_FAILURE);
    }

    struct jpeg_compress_struct cinfo_out;
    struct jpeg_error_mgr jerr_out;

    cinfo_out.err = jpeg_std_error(&jerr_out);
    jpeg_create_compress(&cinfo_out);

    jpeg_stdio_dest(&cinfo_out, outfile);

    cinfo_out.image_width = width;
    cinfo_out.image_height = height;
    cinfo_out.input_components = numChannels;
    cinfo_out.in_color_space = JCS_RGB;

    jpeg_set_defaults(&cinfo_out);
    jpeg_set_quality(&cinfo_out, 75, TRUE);
    jpeg_start_compress(&cinfo_out, TRUE);

    while (cinfo_out.next_scanline < cinfo_out.image_height) {
        row_pointer[0] = &image[cinfo_out.next_scanline * rowStride];
        jpeg_write_scanlines(&cinfo_out, row_pointer, 1);
    }

    jpeg_finish_compress(&cinfo_out);
    jpeg_destroy_compress(&cinfo_out);
    fclose(outfile);

    free(image);

    return 0;
}