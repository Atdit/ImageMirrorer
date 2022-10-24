#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <leptonica/allheaders.h>
#include <dirent.h>
#include <sys/stat.h>

#define DIRECTORY "<input directory>"
#define OUTPUT_DIRECTORY "<output directory>"

void copy(PIX *pix_a, PIX *pix_b, int x_a, int y_a, int x_b, int y_b) {
    uint32_t pixel;
    pixGetPixel(pix_a, x_a, y_a, &pixel);
    pixSetPixel(pix_b, x_b, y_b, pixel);
}

void rotate_90_clockwise(PIX *pix) {
    PIX *old_pix = pixCreate((int) pix->w, (int) pix->h, 32);
    if (old_pix == NULL) {
        fprintf(stderr, "pixCreate() failed\n");
        exit(EXIT_FAILURE);
    }
    memcpy(old_pix->data, pix->data, pix->w * pix->h * 4);

    for (int y = 0; y < pix->h; y++) {
        for (int x = 0; x < pix->w; x++) {
            copy(old_pix, pix, x, y, ((int) pix->w - 1 - y), x);
        }
    }

    pixFreeData(old_pix);
}

void mirror_horizontally(PIX *pix) {
    PIX *old_pix = pixCreate((int) pix->w, (int) pix->h, 32);
    if (old_pix == NULL) {
        fprintf(stderr, "pixCreate() failed\n");
        exit(EXIT_FAILURE);
    }
    memcpy(old_pix->data, pix->data, pix->w * pix->h * 4);

    for (int y = 0; y < pix->h; y++) {
        for (int x = 0; x < pix->w; x++) {
            copy(old_pix, pix, x, y, (int) pix->w - 1 - x, y);
        }
    }

    pixFreeData(old_pix);
}

int main() {
    DIR *dir, *output;
    struct dirent *dirent;

    if ((dir = opendir(DIRECTORY)) == NULL) {
        fprintf(stderr, "Failed to open dir\n");
        return EXIT_FAILURE;
    }

    int output_dir_exists = 1;
    if ((output = opendir(OUTPUT_DIRECTORY)) == NULL) {
        output_dir_exists = 0;
    } else {
        closedir(output);
    }

    if (!output_dir_exists && mkdir(OUTPUT_DIRECTORY, S_IFDIR | S_IRWXU | S_IRWXG) != 0) {
        fprintf(stderr, "Failed to create output directory\n");
        return EXIT_FAILURE;
    }

    while ((dirent = readdir(dir)) != NULL) {
        char *name = dirent->d_name;
        if (strcmp(name, ".") == 0) continue;
        if (strcmp(name, "..") == 0) continue;
        if (strstr(name, ".png") == NULL) continue;

        if (chdir(DIRECTORY) != 0) {
            fprintf(stderr, "Failed to change dir\n");
            return EXIT_FAILURE;
        }

        PIX *pix = pixRead(name);
        if (pix->d != 32) {
            fprintf(stderr, "Bitmap depth is %u instead of 32\n", pix->d);
            return EXIT_FAILURE;
        }
        printf("Read %s with dimensions %dx%d\n", name, pix->w, pix->h);

        if (chdir(OUTPUT_DIRECTORY) != 0) {
            fprintf(stderr, "Failed to change dir\n");
            return EXIT_FAILURE;
        }

        mirror_horizontally(pix);
        rotate_90_clockwise(pix);
        rotate_90_clockwise(pix);

        pixWrite(name, pix, IFF_BMP);
        pixFreeData(pix);
    }
    closedir(dir);

    return EXIT_SUCCESS;
}
