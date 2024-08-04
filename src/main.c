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
 *   ✅ arguments
 *   ✅ copy the exported image to xclip
 *   ✅ add bindings for copying to clipboard and saving
 *      🟨 add feedback for these events (notify-send)
 *   ✅ add ability to accept a piped in image
 *   ✅ sanitize input options
 *   ❌ general cleanup
 *      - if anything i feel it got more messy
 *   ✅ do more 'c' things
 *   ✅ make a better export filepath
 *   ✅ figure out if we can export the file to the clipboard
 *   ✅ save the file on exit or something instead
 *
*********************************************************************************************/

#include "raylib.h"
#include "raymath.h"
#include <bits/getopt_core.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define DEFAULT_BUFFER_SIZE 1024

void print_help(const char *prog) {
    printf("Usage: %s -i <input_file_path> [-o <output_file_path>] [-sch]\n", prog);
    printf("\nOptions:\n");
    printf("    -i      : Input file path\n");
    printf("            You can also pipe image into scedit\n");
    printf("            Example: 'cat img.png | scedit'\n");
    printf("    -o      : Output file path (required)\n");
    printf("    -s      : Enable save+copy on exit\n");
    printf("    -c      : Enable copy on exit\n");
    printf("    -h      : Prints this help message\n");
    printf("\nGUI Usage:\n");
    printf("    L Click     : Draw\n");
    printf("    R Click     : Erase area\n");
    printf("    BACKSPACE   : Clear all\n");
    printf("    CTRL+C      : Copy to clipboard\n");
    printf("    CTRL+S      : Save to designated location\n");
}

/**
 * resolve_directory_path(char *)
 *
 * Ensures a directory path exists
 */
int validate_directory_path(const char *path) {
    struct stat statbuf;
    if (stat(path, &statbuf) != 0) {
        fprintf(stderr, "Error: Path '%s' cannot be accessed\n", path);
        return 0;
    }

    if (!S_ISDIR(statbuf.st_mode)) {
        fprintf(stderr, "Error: '%s' is not a directory\n", path);
        return 0;
    }

    return 1;
}

/**
 * validate_file_path(char *)
 *
 * Ensures a file path exist
 */
int validate_file_path(const char *filePath) {
    if (access(filePath, F_OK) == -1) {
        fprintf(stderr, "Error: File '%s' does not exist or cannot be accessed\n", filePath);
        return 0;
    }

    return 1;
}

void copy_to_clipboard(const RenderTexture2D background, const RenderTexture2D dickDrawings) {
    Image baseImg = LoadImageFromTexture(background.texture);
    Image probablyDickDrawings = LoadImageFromTexture(dickDrawings.texture);
    ImageDraw(&baseImg, probablyDickDrawings,
        (Rectangle){0, 0, (float)probablyDickDrawings.width,
                    (float)probablyDickDrawings.height},
        (Rectangle){0, 0, probablyDickDrawings.width,
                    probablyDickDrawings.height},
        WHITE);
    ImageFlipVertical(&baseImg);
    ExportImage(baseImg, "/tmp/scedit.png");
    int result = system("cat /tmp/scedit.png | xclip -selection clipboard -t image/png");
    printf("system() xclip call result: %d\n", result);
    remove("/tmp/scedit.png");
    system("notify-send \"Copied image to clipboard!\" --app-name \"scedit\"");
    UnloadImage(baseImg);
    UnloadImage(probablyDickDrawings);
}

void save_image(const RenderTexture2D background, const RenderTexture2D dickDrawings, const char *savePath) {
    Image baseImg = LoadImageFromTexture(background.texture);
    Image probablyDickDrawings = LoadImageFromTexture(dickDrawings.texture);
        ImageDraw(&baseImg, probablyDickDrawings,
        (Rectangle){0, 0, (float)probablyDickDrawings.width,
                    (float)probablyDickDrawings.height},
        (Rectangle){0, 0, probablyDickDrawings.width,
                    probablyDickDrawings.height},
        WHITE);
    ImageFlipVertical(&baseImg);
    ExportImage(baseImg, "/tmp/scedit.png");
    ExportImage(baseImg, savePath);
    int result = system("cat /tmp/scedit.png | xclip -selection clipboard -t image/png");
    printf("system() xclip call result: %d\n", result);
    remove("/tmp/scedit.png");
    char cmd[1024] = "notify-send \"Copied image to clipboard!\" \"Saved image to ";
    strcat(cmd, savePath);
    strcat(cmd, "\" --app-name \"scedit\"");
    system(cmd);
    UnloadImage(baseImg);
    UnloadImage(probablyDickDrawings);
}

/**
 * load_image_from_pipe()
 *
 * reads in image file data from unix pipe into raylib image
 */
Image load_image_from_pipe() {
    printf("load_image_from_pipe START\n");
    unsigned char *buffer = NULL;
    size_t buffer_size = DEFAULT_BUFFER_SIZE;
    size_t bytesRead;
    Image image = { 0 };
    
    buffer = (unsigned char *)malloc(buffer_size);
    if (buffer == NULL) {
        perror("Memory allocation failed...");
        return image;
    }

    if (isatty(STDIN_FILENO)) {
        printf("No image data was piped...\n");
        return image;
    }

    bytesRead = fread(buffer, 1, buffer_size, stdin);
    printf("First bytesRead: %d", (int)bytesRead);
    if (bytesRead <= 0) {
        printf("No image data was piped in\n");
        free(buffer);
        return image;
    }

    while (bytesRead == buffer_size) {
        buffer_size *= 2;
        buffer = (unsigned char *)realloc(buffer, buffer_size);
        if (buffer == NULL) {
            perror("Memory re-allocation failed...");
            return image;
        }
        bytesRead += fread(buffer + bytesRead, 1, buffer_size - bytesRead, stdin);
    }

    if (buffer_size > 0) {
        image = LoadImageFromMemory(".png", buffer, bytesRead);
        if (image.data == NULL) {
            printf("Failed to load image from memory.\n");
        }
    }

    free(buffer);
    return image;
}

int main(int argc, char *argv[]) {

    // opts variables
    int helpFlag = 0;
    int inputFilepathFlag = 0;
    int outputFilepathFlag = 0;
    int pipedFileFlag = 0;
    int clipboardOnExitFlag = 0;
    int saveOnExitFlag = 0;
    int opt;

    char *inputFilepath = NULL;
    char *outputFilepath = NULL;
    char *tempFilepath = "/tmp/scedit.png";
    Image img = { 0 };

    while ((opt = getopt(argc, argv, "i:o:csh")) != -1) {
        switch (opt) {
            case 'i':
                inputFilepathFlag = 1;
                pipedFileFlag = 0;
                inputFilepath = optarg;
                break;
            case 'o':
                outputFilepathFlag = 1;
                outputFilepath = optarg;
                break;
            case 'c':
                clipboardOnExitFlag = 1;
                break;
            case 's':
                saveOnExitFlag = 1;
                break;
            case 'h':
            default:
                print_help(argv[0]);
                return 0;
        }
    }

    if (!inputFilepathFlag) pipedFileFlag = 1;
    if (!outputFilepathFlag) { // -o is required rn
        printf("-o is required\n");
        print_help(argv[0]);
        exit(EXIT_FAILURE);
    }

    printf("inputFilepath: %s\n", inputFilepath);
    printf("outputFilepath: %s\n", outputFilepath);
    printf("clipboardOnExit: %d", clipboardOnExitFlag);

    // check if file was not provided thru args
    if (inputFilepathFlag) {
        if (!validate_file_path(inputFilepath)) exit(EXIT_FAILURE);
        printf("LoadImage from path\n");
        img = LoadImage(inputFilepath);
    }

    if (!inputFilepathFlag && pipedFileFlag){
        // try to read from stdin
        printf("load_image_from_pipe CALL\n");
        img = load_image_from_pipe();
        if (img.data == NULL) {
            printf("No input provided\n");
            print_help(argv[0]);
            exit(EXIT_FAILURE);
        }
    } else {
        printf("No input provided\n");
        print_help(argv[0]);
        exit(EXIT_FAILURE);
    }

    // Initialization
    const int screenWidth = 800;
    const int screenHeight = 450;

    float brushSize = 10.0f;
    bool mouseWasPressed = false;

    // setting flags that are features disabled otherwise
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(screenWidth, screenHeight, "scedit");

    // loading images. JPG is disabled and i need to recompile raylib to include
    // support.
    //Image img = LoadImage("/tmp/screen.png"); // loads image in cpu(ram)
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

        // LCONTROL + C event - save image temporarily, call xclip to copy to clipboard
        if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_C)) {
            copy_to_clipboard(target, paintingTarget);
        }

        // LCONTROL + S event - save image to designated path
        if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_S)) {
            save_image(target, paintingTarget, outputFilepath);
        }

        // backspace - clear all drawing
        if (IsKeyPressed(KEY_BACKSPACE)) {
            BeginTextureMode(paintingTarget);
                DrawTexture(tex, 0, 0, WHITE);
            EndTextureMode();
        }

        // brush size
        brushSize += GetMouseWheelMove()*5;
        if (brushSize < 2) brushSize = 2;
        if (brushSize > 50) brushSize = 50;

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
    if (saveOnExitFlag) save_image(target, paintingTarget, outputFilepath);
    if (clipboardOnExitFlag && !saveOnExitFlag) copy_to_clipboard(target, paintingTarget);
    UnloadTexture(tex); // texture unloading
    UnloadRenderTexture(target);
    UnloadRenderTexture(paintingTarget);
    CloseWindow();
    return 0;
}
