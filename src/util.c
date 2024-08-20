#include "util.h"
#include "raylib.h"
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int validate_directory_path(const char *path){ 
    struct stat statbuf;
    if (stat(path, &statbuf) != 0) {
        fprintf(stderr, "Error: Path '%s' cannot be accessed.", path);
        return 0;
    }

    if (!S_ISDIR(statbuf.st_mode)) {
        fprintf(stderr, "Error: Path '%s' cannot be accessed\n", path);
        return 0;
    }

    return 1;
}

int validate_file_path(const char *filePath){ 
    if (access(filePath, F_OK) == -1){
        fprintf(stderr, "Error: File '%s' does not exist or cannot be accessed\n", filePath);
        return 0;
    }

    return 1;
}

void copy_to_clipboard(const RenderTexture2D background, const RenderTexture2D dickDrawings){
    Image backgroundImage = LoadImageFromTexture(background.texture);
    Image paintingImage = LoadImageFromTexture(dickDrawings.texture);
    ImageDraw(&backgroundImage, paintingImage,
        (Rectangle){0, 0, (float)paintingImage.width,
                    (float)paintingImage.height},
        (Rectangle){0, 0, paintingImage.width,
                    paintingImage.height},
    WHITE);

    ImageFlipVertical(&backgroundImage);
    ExportImage(backgroundImage, "/tmp/scedit.png");
    system("cat /tmp/scedit.png | xclip -selection clipboard -t image/png");
    remove("/tmp/scedit.png");
    UnloadImage(backgroundImage);
    UnloadImage(paintingImage);
}

void save_image(const RenderTexture2D background, const RenderTexture2D dickDrawings, const char *savePath){
    Image backgroundImage = LoadImageFromTexture(background.texture);
    Image paintingImage = LoadImageFromTexture(dickDrawings.texture);
    ImageDraw(&backgroundImage, paintingImage,
               (Rectangle){0, 0, (float)paintingImage.width, (float)paintingImage.height},
               (Rectangle){0, 0, paintingImage.width, paintingImage.height}, WHITE);
    ImageFlipVertical(&backgroundImage);
    ExportImage(backgroundImage, savePath);
    char notify_send[1024] = "notify-send \"Saved image to\" \"";
    strcat(notify_send, savePath);
    strcat(notify_send, "\" --app-name \"scedit\"");
    system(notify_send);
    UnloadImage(backgroundImage);
    UnloadImage(paintingImage);
}

Image load_image_from_pipe(){
    printf("TOP load_image_from_pipe()\n");
    unsigned char *buffer = NULL;
    size_t buffer_size = 1024;
    size_t bytesRead;
    Image image = { 0 };

    buffer = (unsigned char *)malloc(buffer_size);
    if (buffer == NULL) {
        perror("Memory allocation failed...\n");
        free(buffer);
        return image;
    }

    // check if input infact exists
    if (isatty(STDIN_FILENO)) {
        printf("Nothing being piped to stdin\n");
        free(buffer);
        return image;
    }

    bytesRead = fread(buffer, 1, buffer_size, stdin);
    printf("First bytes read: %d\n", (int)bytesRead);
    if (bytesRead <= 0) {
        printf("No data was passed into stdin\n");
        free(buffer);
        return image;
    }

    while (bytesRead == buffer_size) {
        buffer_size *= 2;
        buffer = (unsigned char *)realloc(buffer, buffer_size);
        if (buffer == NULL) {
            perror("Memory re-allocation failed...\n");
            free(buffer);
            return image;
        }

        bytesRead += fread(buffer + bytesRead, 1, buffer_size - bytesRead, stdin);
    }

    if (buffer_size > 0) {
        image = LoadImageFromMemory(".png", buffer, bytesRead);
        if (image.data == NULL) printf("Failed to load image from memory (stdin)\n");
    }

    free(buffer);
    return image;
}

void str_hex_to_rgb_colour(const char *choice, unsigned char *r, unsigned char *g, unsigned char *b){
    unsigned long hex_value = strtoul(choice, NULL, 16);
    *r = (hex_value >> 16) & 0xFF;
    *g = (hex_value >> 8) & 0xFF;
    *b = hex_value & 0xFF;
}
