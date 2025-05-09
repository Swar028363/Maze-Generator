#include "bmp.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct BmpFileHeader BmpFileHeader;
typedef struct BmpInfoHeader BmpInfoHeader;
typedef struct RGBTriple RGBTriple;
typedef struct BmpImage BmpImage;


int calculate_padding(int width) {
    return (4 - (width * sizeof(RGBTriple)) % 4) % 4; // Calculate padding for each row
}


BmpImage *bmp_create(int width, int height) {
int padding = calculate_padding(width);
    int rowSize = width * sizeof(RGBTriple) + padding;
    int pixelDataSize = rowSize * height;
    int fileSize = sizeof(BmpFileHeader) + sizeof(BmpInfoHeader) + pixelDataSize;

    BmpImage *image = malloc(sizeof(BmpImage));
    if (!image) return NULL;

    image->fileHeader.bfType = 0x4D42;
    image->fileHeader.bfSize = fileSize;
    image->fileHeader.bfReserved1 = 0;
    image->fileHeader.bfReserved2 = 0;
    image->fileHeader.bfOffBits = sizeof(BmpFileHeader) + sizeof(BmpInfoHeader);

    image->infoHeader.biSize = sizeof(BmpInfoHeader);
    image->infoHeader.biWidth = width;
    image->infoHeader.biHeight = height;
    image->infoHeader.biPlanes = 1;
    image->infoHeader.biBitCount = 24;
    image->infoHeader.biCompression = 0;
    image->infoHeader.biSizeImage = pixelDataSize;
    image->infoHeader.biXPelsPerMeter = 2835; // 72 DPI
    image->infoHeader.biYPelsPerMeter = 2835;
    image->infoHeader.biClrUsed = 0;
    image->infoHeader.biClrImportant = 0;

    image->pixels = malloc(width * height * sizeof(RGBTriple));
    if (!image->pixels) {
        free(image);
        return NULL;
    }

    // Initialize all pixels to white
    for (int i = 0; i < width * height; i++) {
        image->pixels[i].rgbtRed = 255;
        image->pixels[i].rgbtGreen = 255;
        image->pixels[i].rgbtBlue = 255;
    }

    image->padding = padding;

    return image;
}

BmpImage *bmp_load(const char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) return NULL;

    BmpFileHeader fileHeader;
    fread(&fileHeader, sizeof(BmpFileHeader), 1, fp);
    if (fileHeader.bfType != 0x4D42) { // 'BM' in little-endian
        fclose(fp);
        return NULL;
    }

    BmpInfoHeader infoHeader;
    fread(&infoHeader, sizeof(BmpInfoHeader), 1, fp);
    if (infoHeader.biBitCount != 24 || infoHeader.biCompression != 0) {
        fclose(fp);
        return NULL;
    }

    int width = infoHeader.biWidth;
    int height = abs(infoHeader.biHeight);
    int padding = calculate_padding(width);

    RGBTriple *pixels = malloc(width * height * sizeof(RGBTriple));
    if (!pixels) {
        fclose(fp);
        return NULL;
    }

    fseek(fp, fileHeader.bfOffBits, SEEK_SET);
    for (int y = height - 1; y >= 0; y--) {
        fread(&pixels[y * width], sizeof(RGBTriple), width, fp);
        fseek(fp, padding, SEEK_CUR);
    }

    fclose(fp);

    BmpImage *image = malloc(sizeof(BmpImage));
    if (!image) {
        free(pixels);
        return NULL;
    }

    image->fileHeader = fileHeader;
    image->infoHeader = infoHeader;
    image->pixels = pixels;
    image->padding = padding;

    return image;
}

bool bmp_save(const char *filename, const BmpImage *image) {
    FILE* fp = fopen(filename, "wb");
    if (!fp) return false;

    fwrite(&image->fileHeader, sizeof(BmpFileHeader), 1, fp);
    fwrite(&image->infoHeader, sizeof(BmpInfoHeader), 1, fp);
    
    int width = image->infoHeader.biWidth;
    int height = abs(image->infoHeader.biHeight);
    int padding = image->padding;
    uint8_t pad[3] = {0, 0, 0};
    
    for (int y = height - 1; y >= 0; y--) {
        fwrite(&image->pixels[y * width], sizeof(RGBTriple), width, fp);
        fwrite(pad, 1, padding, fp);
    }

    fclose(fp);
    return true;
}

void bmp_free(BmpImage *image) {
    if (image) {
        free(image->pixels);
        free(image);
    }
}

void bmp_set_pixel(BmpImage *image, int x, int y, RGBTriple color) {
    int width = image->infoHeader.biWidth;
    int height = abs(image->infoHeader.biHeight);
    if (x < 0 || x >= width || y < 0 || y >= height) return;
    image->pixels[y * width + x] = color;
}

RGBTriple* bmp_get_pixel(const BmpImage *image, int x, int y) {
    int width = image->infoHeader.biWidth;
    int height = abs(image->infoHeader.biHeight);
    if (x < 0 || x >= width || y < 0 || y >= height) {
        return NULL;
    }
    return &image->pixels[y * width + x];
}