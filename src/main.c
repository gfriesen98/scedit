/*******************************************************************************************
 *
 *   sceditor - screenshot drawing utility
 *
 *   c/raylib learning project
 *
 *   using raylib examples to get what i want:
 *   - raylib [core] example - Basic window
 *   - raylib [textures] example - Image loading and drawing on it
 *   - raylib [textures] example - Mouse painting
 *   - raylib [core] example - window scale letterbox (and virtual mouse)
 *
 *--------------------------------------------------------------------------------------------
 * TODO: things to figure out
 *   - arguments
 *   - copy the exported image to xclip
 *   - general cleanup
 *   - do more 'c' things
 *   - make a better export filepath
 *   - figure out if we can export the file to the clipboard
 *   - save the file on exit or something instead
 *
*********************************************************************************************/

#include "raylib.h"
#include "raymath.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

void print_help() {
    printf("Usage: sceditor [options]\n");
    printf("\nIf no options are supplied, program will only copy to clipboard\n");
    printf("\nOptions:\n");
    printf("    -i      : Input file path\n");
    printf("    -o      : Output file path\n");
    printf("    -h      : Prints this help message\n");
    printf("\nGUI Usage:\n");
    printf("    L Click : Draw\n");
    printf("    R Click : Erase area\n");
    printf("    C       : Clear all\n");
}

void resolve_path(const char *output_path) {
    
}

int main(int argc, char *argv[]) {

    int helpFlag = 0;
    int inputFilepathFlag = 0;
    int outputFilepathFlag = 0;
    char *inputFilepath = "/tmp/screen.png";
    char *outputFilepath = getenv("HOME");

    int l_click = 0;
    int r_click = 1;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-h") == 0) {
            print_help();
            return 0;
        }

        if (strcmp(argv[i], "-i") == 0 && i + 1 < argc) {
            inputFilepath = argv[i + 1];
            i++;
        }

        if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            outputFilepath = argv[i + 1];
            i++;
        }
    }

    printf("inputFilepath: %s\n", inputFilepath);
    printf("outputFilepath: %s\n", outputFilepath);

    // Initialization
    const int screenWidth = 800;
    const int screenHeight = 450;

    float brushSize = 10.0f;
    bool mouseWasPressed = false;

    // setting flags that are features disabled otherwise
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(screenWidth, screenHeight, "scweenshottah");

    // loading images. JPG is disabled and i need to recompile raylib to include
    // support.
    Image img = LoadImage("/tmp/screen.png"); // loads image in cpu(ram)
    Texture2D tex = LoadTextureFromImage(img); // Image converted to texture, uploaded to GPU memory (vram)
    UnloadImage(img); // Once image is uploaded to vram it can be unloaded from ram

    // for letterbox scaling
    int progScreenWidth = tex.width;
    int progScreenHeight = tex.height;
    RenderTexture2D target = LoadRenderTexture(progScreenWidth, progScreenHeight);
    RenderTexture2D paintingTarget = LoadRenderTexture(progScreenWidth, progScreenHeight);
    SetTextureFilter(target.texture, TEXTURE_FILTER_BILINEAR);

    SetWindowMinSize(250, 250);
    SetWindowPosition(0, 0);
    SetWindowMonitor(0); // idk if this means its primary monitor or not...
    // SetTargetFPS(165);   // Set our game to run at 60 frames-per-second

    // clear buffer before drawing
    BeginTextureMode(target);
    ClearBackground(RAYWHITE);
    EndTextureMode();

    // Main loop
    while (!WindowShouldClose()) {
        // update variables

        // Scaling
        float scale = MIN((float)GetScreenWidth() / progScreenWidth, (float)GetScreenHeight() / progScreenHeight);
        Vector2 mouse = GetMousePosition();
        Vector2 virtualMouse = {0};
        virtualMouse.x = (mouse.x - (GetScreenWidth() - (progScreenWidth * scale)) * 0.5f) / scale;
        virtualMouse.y = (mouse.y - (GetScreenHeight() - (progScreenHeight * scale)) * 0.5f) / scale;
        virtualMouse = Vector2Clamp(virtualMouse, (Vector2){0, 0}, (Vector2){(float)progScreenWidth, (float)progScreenHeight});

        // Left mouse button event - mouse pointer painting
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            BeginTextureMode(paintingTarget);
                DrawCircle((int)virtualMouse.x, (int)virtualMouse.y, brushSize, RED);
            EndTextureMode();
        }

        // Right mouse button event - mouse pointer erasing
        // erases by redrawing background texture pixels over the painting texture
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
            float eraseScale = brushSize;
            BeginTextureMode(paintingTarget);
                Rectangle sourceRec = { virtualMouse.x - eraseScale / 2, virtualMouse.y - eraseScale / 2, eraseScale, eraseScale };
                Rectangle destRec = { virtualMouse.x - eraseScale / 2, virtualMouse.y - eraseScale / 2, eraseScale, eraseScale };
                DrawTexturePro(tex, sourceRec, destRec, (Vector2){ 0, 0 }, 0.0f, WHITE);
            EndTextureMode();
        }

        // Spacebar pressed event - save image, call xclip to copy to clipboard
        if (IsKeyPressed(KEY_SPACE)) {
            Image baseImg = LoadImageFromTexture(target.texture);
            Image probablyDickDrawings = LoadImageFromTexture(paintingTarget.texture);
            ImageDraw(&baseImg, probablyDickDrawings,
                (Rectangle){0, 0, (float)probablyDickDrawings.width,
                            (float)probablyDickDrawings.height},
                (Rectangle){0, 0, probablyDickDrawings.width,
                            probablyDickDrawings.height},
                WHITE);
            ImageFlipVertical(&baseImg);
            ExportImage(baseImg, "/home/gort/images/MyAwesomeImage.png");
            int result = system("cat /home/gort/images/MyAwesomeImage.png | xclip -selection clipboard -t image/png");
            printf("system() xclip call result: %d\n", result);
            UnloadImage(baseImg);
            UnloadImage(probablyDickDrawings);
        }

        if (IsKeyPressed(KEY_C)) {
            BeginTextureMode(paintingTarget);
                DrawTexture(tex, 0, 0, WHITE);
            EndTextureMode();
        }

        // Draw
        BeginTextureMode(target);
            DrawTexture(tex, 0, 0, WHITE);
        EndTextureMode();

        BeginDrawing();

        ClearBackground(BLACK);

        // Draw background texture, to scale
        DrawTexturePro(
            target.texture,
            (Rectangle){0.0f, 0.0f, (float)target.texture.width,
                        (float)-target.texture.height},
            (Rectangle){
                (GetScreenWidth() - ((float)progScreenWidth * scale)) * 0.5f,
                (GetScreenHeight() - ((float)progScreenHeight * scale)) * 0.5f,
                (float)progScreenWidth * scale, (float)progScreenHeight * scale},
            (Vector2){0, 0}, 0.0f, WHITE);

        // Draw probably dicks, to scale
        DrawTexturePro(
            paintingTarget.texture,
            (Rectangle){0.0f, 0.0f, (float)paintingTarget.texture.width,
                    (float)-paintingTarget.texture.height},
            (Rectangle){
                (GetScreenWidth() - ((float)progScreenWidth * scale)) * 0.5f,
                (GetScreenHeight() - ((float)progScreenHeight * scale)) * 0.5f,
                (float)progScreenWidth * scale, (float)progScreenHeight * scale},
            (Vector2){0, 0}, 0.0f, WHITE);

        // Draw outline as a pointer
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
            DrawCircleLines((int)mouse.x, (int)mouse.y, brushSize, GRAY);
        } else {
            DrawCircle(GetMouseX(), GetMouseY(), brushSize, RED);
        }

        EndDrawing();
  }

  // De-Initialization
  UnloadTexture(tex); // texture unloading
  UnloadRenderTexture(target);
  UnloadRenderTexture(paintingTarget);
  CloseWindow();

  return 0;
}
