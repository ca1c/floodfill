#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <jpeglib.h>

// store RGB values
typedef struct {
    unsigned char r, g, b;
} RGB;

bool colorsContainsColor(RGB * colorsArr, RGB color, int colorsArrSize) {
    for(int i = 0; i < colorsArrSize; i++) {
        if(colorsArr[i].r == color.r && colorsArr[i].g == color.g && colorsArr[i].b == color.b) {
            return true;
        }
    }
    return false;
}

int getColorIndex(RGB * colorsArr, RGB color, int colorsArrSize) {
    for(int i = 0; i < colorsArrSize; i++) {
        if(colorsArr[i].r == color.r && colorsArr[i].g == color.g && colorsArr[i].b == color.b) {
            return i;
        }
    }
    return -1;
}

// find the most common color in the image
RGB findMostCommonColor(JSAMPLE* image, int width, int height) {
    size_t array_size = 255 * 255 * 255;

    int j = 0;
    RGB * colorsArr = (RGB *)malloc(array_size * sizeof(RGB));
    int * colorsCount = (int *)malloc(array_size * sizeof(int));

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int index = y * width * 3 + x * 3;
            RGB color = {image[index], image[index + 1], image[index + 2]};
            // printf("[%d, %d, %d]\n", color.r, color.g, color.b);
            if(colorsContainsColor(colorsArr, color, j)) {
                colorsCount[getColorIndex(colorsArr, color, j)]++;
            }
            else {
                colorsArr[j] = color;
                colorsCount[j] = 1;
                j++;
            }
        }
    }

    int maxCount = 0;
    int maxIndex = 0;
    RGB mostCommonColor;

    for (int i = 0; i < j; i++) {
        if (colorsCount[i] > maxCount) {
            maxCount = colorsCount[i];
            maxIndex = i;
        }
    }

    mostCommonColor.r = colorsArr[maxIndex].r;
    mostCommonColor.g = colorsArr[maxIndex].g;
    mostCommonColor.b = colorsArr[maxIndex].b;

    return mostCommonColor;
}

// replace the most common color with a new color
void replaceColor(JSAMPLE* image, int width, int height, RGB oldColor, RGB newColor) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Since image is a 1 dimensional array, finding the specific pixel requires some math
            // y * width gives the current row of the image if each pixel only had one value
            // since each pixel has three values for rgb we have to multiply this by 3
            // then to get the current pixel in that row, add x for the single pixel column
            // then multiply by 3 because of the 3 values for each pixel rgb
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

    printf("Doing floodfill for image %s\n", argv[1]);
    printf("New color RGB: %d, %d, %d\n", newColor.r, newColor.g, newColor.b);

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

    printf("Finding most common color...\n");

    // Find the most common color in the image
    RGB mostCommonColor = findMostCommonColor(image, width, height);
    printf("Most common color RGB: %d, %d, %d\n", mostCommonColor.r, mostCommonColor.g, mostCommonColor.b);

    printf("Replacing most common color with new color...\n");

    // Replace the most common color with the new color
    replaceColor(image, width, height, mostCommonColor, newColor);

    printf("Compressing...\n");

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

    printf("Done! output filename: %s\n", argv[2]);

    return 0;
}