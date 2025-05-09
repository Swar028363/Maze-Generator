#ifndef BMP_H
#define BMP_H

#include <stdint.h>

struct BmpFileHeader {
    uint16_t bfType; // Magic number for BMP files (should be 'BM')
    uint32_t bfSize; // Size of the BMP file in bytes
    uint16_t bfReserved1; // Reserved; must be 0
    uint16_t bfReserved2; // Reserved; must be 0
    uint32_t bfOffBits; // Offset to the start of pixel data
} __attribute__((packed)); // Ensure no padding between members

struct BmpInfoHeader {
    uint32_t biSize; // Size of this header (40 bytes)
    int32_t  biWidth; // Width of the bitmap in pixels
    int32_t  biHeight; // Height of the bitmap in pixels
    uint16_t biPlanes; // Number of color planes (must be 1)
    uint16_t biBitCount; // Bits per pixel (1, 4, 8, or 24)
    uint32_t biCompression; // Compression type (0 = uncompressed)
    uint32_t biSizeImage; // Size of the pixel data (can be 0 for uncompressed)
    int32_t  biXPelsPerMeter; // Horizontal resolution (pixels per meter)
    int32_t  biYPelsPerMeter; // Vertical resolution (pixels per meter)
    uint32_t biClrUsed; // Number of colors in the color palette
    uint32_t biClrImportant; // Important colors (0 = all colors are important)
} __attribute__((packed)); // Ensure no padding between members

struct RGBTriple {
    uint8_t rgbtBlue; // Blue component (0-255)
    uint8_t rgbtGreen; // Green component (0-255)
    uint8_t rgbtRed; // Red component (0-255)
} __attribute__((packed)); // Ensure no padding between members

struct BmpImage {
    struct BmpFileHeader fileHeader; // BMP file header
    struct BmpInfoHeader infoHeader; // DIB header
    struct RGBTriple *pixels; // Pixel data (bottom-up)
    int padding; // Row padding in bytes (0-3)
};

struct BmpImage *bmp_create(int width, int height); // Create a new BMP image

struct BmpImage *bmp_load(const char *filename); // Load a BMP file from disk

bool bmp_save(const char *filename, const struct BmpImage *image); // Save a BMP file to disk

void bmp_free(struct BmpImage *image); // Free the memory used by a BMP image

void bmp_set_pixel(struct BmpImage *image, int x, int y, struct RGBTriple color); // Set a pixel's color

struct RGBTriple* bmp_get_pixel(const struct BmpImage *image, int x, int y); // Get a pixel's color

#endif // BMP_H