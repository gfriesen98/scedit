#ifndef UTIL_H
#define UTIL_H

#include "raylib.h"

int validate_directory_path(const char *path);
int validate_file_path(const char *filePath);

void copy_to_clipboard(const RenderTexture2D background, const RenderTexture2D dickDrawings);
void save_image(const RenderTexture2D background, const RenderTexture2D dickDrawings, const char *savePath);
Image load_image_from_pipe();
void str_hex_to_rgb_colour(const char *choice, unsigned char *r, unsigned char *g, unsigned char *b);

#endif // UTIL_H
