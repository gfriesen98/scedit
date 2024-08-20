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
*********************************************************************************************/

#include "raylib.h"
#include "raymath.h"
#include "util.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

/**
 * print_help(char prog)
 * Displays the help message
 */
void print_help(const char *prog) {
    printf("Usage: %s -i <input_file_path> -o <output_file_path> [-bclmsxh]\n", prog);
    printf("\nOptions:\n");
    printf("    [stdin] -i --input       : Input file path\n");
    printf("                        You can also pipe image into scedit\n");
    printf("                        Example: 'cat img.png | scedit -o myimg.png'\n");
    printf("    -o --output              : Output file path (required)\n");
    printf("    -b --brushsize           : Brush size (default '5')\n");
    printf("    -c --colour              : Brush colour or hex colour code. Default 'red'\n");
    printf("                        Example: 'scedit -c white'\n");
    printf("                                 'scedit -c fcba03'\n");
    printf("    -l --listcolours         : List all builtin colours\n");
    printf("    -m --monitor             : Display to open on (default 0)\n");
    printf("    -s --saveonexit          : Enable save+copy on exit (Default disabled)\n");
    printf("    -x --clipboardonexit     : Enable copy on exit (Default disabled)\n");
    printf("    -h --help                : Prints this help message\n");
    printf("\nGUI Usage:\n");
    printf("    L Click         : Draw\n");
    printf("    R Click         : Erase area\n");
    printf("    Scroll up/down  : Change brush size\n");
    printf("    Backspace       : Clear all\n");
    printf("    Ctrl+C          : Copy to clipboard\n");
    printf("    Ctrl+S          : Save to output path and copy\n");
}

/**
 * select_colour()
 * Returns a raylib colour from --colour choice
 */
Color select_colour(const char *choice) {
    if (strcmp(choice, "white") == 0) return RAYWHITE;
    else if (strcmp(choice, "yellow") == 0) return YELLOW;
    else if (strcmp(choice, "gold") == 0) return GOLD;
    else if (strcmp(choice, "orange") == 0) return ORANGE;
    else if (strcmp(choice, "red") == 0) return RED;
    else if (strcmp(choice, "maroon") == 0) return MAROON;
    else if (strcmp(choice, "green") == 0) return GREEN;
    else if (strcmp(choice, "lime") == 0) return LIME;
    else if (strcmp(choice, "darkgreen") == 0) return DARKGREEN;
    else if (strcmp(choice, "skyblue") == 0) return SKYBLUE;
    else if (strcmp(choice, "blue") == 0) return BLUE;
    else if (strcmp(choice, "darkblue") == 0) return DARKBLUE;
    else if (strcmp(choice, "purple") == 0) return PURPLE;
    else if (strcmp(choice, "violet") == 0) return VIOLET;
    else if (strcmp(choice, "darkpurple") == 0) return DARKPURPLE;
    else if (strcmp(choice, "beige") == 0) return BEIGE;
    else if (strcmp(choice, "brown") == 0) return BROWN;
    else if (strcmp(choice, "darkbrown") == 0) return DARKBROWN;
    else if (strcmp(choice, "lightgray") == 0 || strcmp(choice, "lightgrey") == 0) return LIGHTGRAY;
    else if (strcmp(choice, "gray") == 0 || strcmp(choice, "grey") == 0) return GRAY;
    else if (strcmp(choice, "darkgray") == 0 || strcmp(choice, "darkgrey") == 0) return DARKGRAY;
    else if (strcmp(choice, "black") == 0) return BLACK;
    else {
        unsigned char r,g,b;
        str_hex_to_rgb_colour(choice, &r, &g, &b);
        return (Color){r,g,b, 255};
    }
}

int main(int argc, char *argv[]) {
    // default screen size - for init
    const int screenWidth = 800;
    const int screenHeight = 450;

    // Opts flags
    int opt;
    int inputFilepathFlag = 0;
    int outputFilepathFlag = 0;
    int pipedFileFlag = 0;
    int clipboardOnExitFlag = 0;
    int saveOnExitFlag = 0;

    // Opts variables
    char *inputFilepath = NULL;
    char *outputFilepath = NULL;
    int monitor = 0;
    float brushSize = 5.0f;
    Color colourSelected = RED; // Default RED

    Image inputImage = { 0 }; // init screenshot here so we can determine how its being loaded
    
    static struct option long_options[] = {
        {"input", required_argument, 0, 'i'},
        {"output", required_argument, 0, 'o'},
        {"monitor", required_argument, 0, 'm'},
        {"brushsize", required_argument, 0, 'b'},
        {"colour", required_argument, 0, 'c'},
        {"listcolours", no_argument, 0, 'l'},
        {"clipboardonexit", no_argument, 0, 'x'},
        {"saveonexit", no_argument, 0, 's'},
        {"help", no_argument, 0, 'h'},
        {0,0,0,0}
    };

    // Command line options
    while ((opt = getopt_long(argc, argv, "i:o:m:b:c:lxsh", long_options, NULL)) != -1) {
        switch (opt) {
            case 'i':   // Input file path
                inputFilepathFlag = 1;
                pipedFileFlag = 0;
                inputFilepath = optarg;
                break;
            case 'o':   // Output file path
                outputFilepathFlag = 1;
                outputFilepath = optarg;
                break;
            case 'm':   // Monitor index
                monitor = atoi(optarg);
                break;
            case 'b':   // Brush size
                brushSize = atof(optarg);
                break;
            case 'c':   // Colour
                colourSelected = select_colour(optarg);
                break;
            case 'l':   // List colours
                printf("Available colours:\n");
                printf("white, yellow, gold, orange,\nred, maroon, green, lime\ndarkgreen, skyblue, blue, darkblue,\npurple, violet, darkpurple, beige,\nbrown, darkbrown, lightgre/ay, gre/ay,\ndarkgre/ay, black\n");
                exit(EXIT_SUCCESS);
                break;
            case 'x':   // Enable clipboard save on exit
                clipboardOnExitFlag = 1;
                break;
            case 's':   // Enable save on exit
                saveOnExitFlag = 1;
                break;
            case 'h':   // Show help message
            default:
                print_help(argv[0]);
                return 0;
        }
    }

    // Set piped file flag if -i isnt provided
    if (!inputFilepathFlag) pipedFileFlag = 1;

    // Check if output file path is provided
    if (!outputFilepathFlag) { // -o is required rn
        printf("-o is required\n");
        print_help(argv[0]);
        exit(EXIT_FAILURE);
    }

    // Check if input file provided with -i is valid
    if (inputFilepathFlag) {
        if (!validate_file_path(inputFilepath)) exit(EXIT_FAILURE);
        printf("LoadImage from path\n");
        inputImage = LoadImage(inputFilepath);
    }

    // Check if file was piped in
    if (!inputFilepathFlag && pipedFileFlag){
        // Try to read from stdin
        printf("Open load_image_from_pipe()\n");
        inputImage = load_image_from_pipe();
        if (inputImage.data == NULL) {
            printf("No input provided\n");
            print_help(argv[0]);
            exit(EXIT_FAILURE);
        }
    } else { // If -i or pipe not provided then exit
        printf("No input provided\n");
        print_help(argv[0]);
        exit(EXIT_FAILURE);
    }

    // Raylib initialization
    // Get input image dimensions to eventually set window size with
    int imgWidth = inputImage.width;
    int imgHeight = inputImage.height;
    
    // setting flags that are features disabled otherwise
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(screenWidth, screenHeight, "scedit");

    // Convert input image to texture
    Texture2D inputTexture = LoadTextureFromImage(inputImage); // image converted to texture, uploaded to GPU memory (vram)
    UnloadImage(inputImage); // once image is uploaded to vram it can be unloaded from ram

    // Variables for letterbox scaling
    int progScreenWidth = inputTexture.width;
    int progScreenHeight = inputTexture.height;

    // Set up RenderTextures for drawing and scaling
    RenderTexture2D backgroundTarget = LoadRenderTexture(progScreenWidth, progScreenHeight);
    RenderTexture2D paintingTarget = LoadRenderTexture(progScreenWidth, progScreenHeight);
    SetTextureFilter(backgroundTarget.texture, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(paintingTarget.texture, TEXTURE_FILTER_BILINEAR);

    // Window settings
    SetWindowMinSize(250, 250);
    SetWindowPosition(0, 0);
    SetWindowMonitor(monitor); // 0 for primary monitor?
    // SetTargetFPS(60)
    
    // Get monitor resolution to help determine setting window size
    const int monitorWidth = GetMonitorWidth(monitor);
    const int monitorHeight = GetMonitorHeight(monitor);

    // Check if image loaded is larger than the monitor or smaller than the minimum window size
    if (imgWidth >= monitorWidth) imgWidth = monitorWidth - 40;
    if (imgHeight >= monitorHeight) imgHeight = monitorHeight - 40;
    if (imgWidth <= 250) imgWidth = 250;
    if (imgHeight <= 250) imgHeight = 250;
    SetWindowSize(imgWidth, imgHeight);
    //SetWindowMinSize(imgWidth, imgHeight);

    // Clear buffer before drawing
    BeginTextureMode(backgroundTarget);
        ClearBackground(RAYWHITE);
    EndTextureMode();

    // Main loop
    while (!WindowShouldClose()) {
        // Scaling + mouse settings
        float scale = MIN((float)GetScreenWidth() / progScreenWidth, (float)GetScreenHeight() / progScreenHeight);
        Vector2 mouse = GetMousePosition();
        Vector2 virtualMouse = {0};
        virtualMouse.x = (mouse.x - (GetScreenWidth() - (progScreenWidth * scale)) * 0.5f) / scale;
        virtualMouse.y = (mouse.y - (GetScreenHeight() - (progScreenHeight * scale)) * 0.5f) / scale;
        virtualMouse = Vector2Clamp(virtualMouse, (Vector2){0, 0}, (Vector2){(float)progScreenWidth, (float)progScreenHeight});

        // Left mouse button event - mouse pointer painting
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            BeginTextureMode(paintingTarget);
                DrawCircle((int)virtualMouse.x, (int)virtualMouse.y, brushSize / scale, colourSelected);
            EndTextureMode();
        }

        // Right mouse button event - mouse pointer erasing
        // erases by redrawing background texture pixels over the painting texture
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
            float eraseScale = brushSize;
            BeginTextureMode(paintingTarget);
                Rectangle sourceRec = { virtualMouse.x - eraseScale / 2, virtualMouse.y - eraseScale / 2, eraseScale, eraseScale };
                Rectangle destRec = { virtualMouse.x - eraseScale / 2, virtualMouse.y - eraseScale / 2, eraseScale, eraseScale };
                DrawTexturePro(inputTexture, sourceRec, destRec, (Vector2){ 0, 0 }, 0.0f, WHITE);
            EndTextureMode();
        }

        // LCONTROL + C event - save image temporarily, call xclip to copy to clipboard
        if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_C)) {
            copy_to_clipboard(backgroundTarget, paintingTarget);
        }

        // LCONTROL + S event - save image to designated path
        if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_S)) {
            save_image(backgroundTarget, paintingTarget, outputFilepath);
        }

        // Backspace event - clear all drawing
        if (IsKeyPressed(KEY_BACKSPACE)) {
            BeginTextureMode(paintingTarget);
                DrawTexture(inputTexture, 0, 0, WHITE);
            EndTextureMode();
        }

        // Scroll up/down event - Brush size
        brushSize += GetMouseWheelMove()*2;
        if (brushSize < 2) brushSize = 2;
        if (brushSize > 50) brushSize = 50;

        // Render image on background target before drawing
        BeginTextureMode(backgroundTarget);
            DrawTexture(inputTexture, 0, 0, WHITE);
        EndTextureMode();

        // Draw stuff to the screen
        BeginDrawing();

            ClearBackground(BLACK);

            // Draw background texture, to scale
            DrawTexturePro(
                backgroundTarget.texture,
                (Rectangle){
                    0.0f,
                    0.0f,
                    (float)backgroundTarget.texture.width,
                    (float)-backgroundTarget.texture.height},
                (Rectangle){
                    (GetScreenWidth() - ((float)progScreenWidth * scale)) * 0.5f,
                    (GetScreenHeight() - ((float)progScreenHeight * scale)) * 0.5f,
                    (float)progScreenWidth * scale,
                    (float)progScreenHeight * scale
                },
                (Vector2){0, 0}, 0.0f, WHITE);

            // Draw probably dicks on the painting target, to scale
            DrawTexturePro(
                paintingTarget.texture,
                (Rectangle){
                    0.0f,
                    0.0f,
                    (float)paintingTarget.texture.width,
                    (float)-paintingTarget.texture.height
                },
                (Rectangle){
                    (GetScreenWidth() - ((float)progScreenWidth * scale)) * 0.5f,
                    (GetScreenHeight() - ((float)progScreenHeight * scale)) * 0.5f,
                    (float)progScreenWidth * scale,
                    (float)progScreenHeight * scale
                },
                (Vector2){0, 0}, 0.0f, WHITE);

            // Draw circle outline for brush mouse pointer
            if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
                DrawCircleLines((int)mouse.x, (int)mouse.y, brushSize, GRAY);
            } else {
                DrawCircle(GetMouseX(), GetMouseY(), brushSize, colourSelected);
            }

        EndDrawing();
    }

    // Check if program should save/copy on exit
    if (saveOnExitFlag) save_image(backgroundTarget, paintingTarget, outputFilepath);
    if (clipboardOnExitFlag) copy_to_clipboard(backgroundTarget, paintingTarget);

    // De-initialization
    UnloadTexture(inputTexture); // texture unloading
    UnloadRenderTexture(backgroundTarget);
    UnloadRenderTexture(paintingTarget);
    CloseWindow();
    return 0;
}
