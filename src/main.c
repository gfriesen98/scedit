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
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define DEFAULT_BUFFER_SIZE 1024

/**
 * print_help(char prog)
 *
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

/**
 * copy_to_clipboard(RenderTexture2D background, RenderTexture2D dickDrawings)
 *
 * Copies the modified image to the clipboard with xclip
 */
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

/**
 * save_image(RenderTexture2D background, const RenderTexture2D dickDrawings, char *savePath)
 *
 * Saves the modified image to the designated path and copy to clipboard
 */
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
    //ExportImage(baseImg, "/tmp/scedit.png");
    ExportImage(baseImg, savePath);
    //int result = system("cat /tmp/scedit.png | xclip -selection clipboard -t image/png");
    //printf("system() xclip call result: %d\n", result);
    //remove("/tmp/scedit.png");
    char cmd[1024] = "notify-send \"Saved image to\" \"";
    strcat(cmd, savePath);
    strcat(cmd, "\" --app-name \"scedit\"");
    system(cmd);
    UnloadImage(baseImg);
    UnloadImage(probablyDickDrawings);
}

/**
 * load_image_from_pipe()
 *
 * Reads stdin buffer into raylib image
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

void str_hex_to_rgb_colour(const char *choice, unsigned char *r, unsigned char *g, unsigned char *b) {
    unsigned long hex_value = strtoul(choice, NULL, 16);
    *r = (hex_value >> 16) & 0xFF;
    *g = (hex_value >> 8) & 0xFF;
    *b = hex_value & 0xFF;
}

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

    Image img = { 0 }; // init screenshot here so we can determine how its being loaded
    
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
            case 'm':
                monitor = atoi(optarg);
                break;
            case 'b':
                brushSize = atof(optarg);
                break;
            case 'c':
                colourSelected = select_colour(optarg);
                break;
            case 'l':
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

    if (!inputFilepathFlag) pipedFileFlag = 1;
    if (!outputFilepathFlag) { // -o is required rn
        printf("-o is required\n");
        print_help(argv[0]);
        exit(EXIT_FAILURE);
    }

    printf("inputFilepath: %s\n", inputFilepath);
    printf("outputFilepath: %s\n", outputFilepath);
    printf("clipboardOnExit: %d\n", clipboardOnExitFlag);
    printf("brushSize: %f\n", brushSize);
    printf("monitor: %d\n", monitor);
    //exit(EXIT_SUCCESS);

    // Check if file was not provided thru args
    if (inputFilepathFlag) {
        if (!validate_file_path(inputFilepath)) exit(EXIT_FAILURE);
        printf("LoadImage from path\n");
        img = LoadImage(inputFilepath);
    }

    if (!inputFilepathFlag && pipedFileFlag){
        // Try to read from stdin
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

    // Raylib initialization
    int imgWidth = img.width;
    int imgHeight = img.height;
    printf("img width: %d\nimg height: %d\n", img.width, img.height);
    
    // setting flags that are features disabled otherwise
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(screenWidth, screenHeight, "scedit");

    Texture2D tex = LoadTextureFromImage(img); // image converted to texture, uploaded to GPU memory (vram)
    UnloadImage(img); // once image is uploaded to vram it can be unloaded from ram

    // Variables for letterbox scaling
    int progScreenWidth = tex.width;
    int progScreenHeight = tex.height;
    // Set up RenderTextures for drawing and scaling
    RenderTexture2D target = LoadRenderTexture(progScreenWidth, progScreenHeight);
    RenderTexture2D paintingTarget = LoadRenderTexture(progScreenWidth, progScreenHeight);
    SetTextureFilter(target.texture, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(paintingTarget.texture, TEXTURE_FILTER_BILINEAR);

    // Window settings
    SetWindowMinSize(250, 250);
    SetWindowPosition(0, 0);
    SetWindowMonitor(monitor); // idk if this means its primary monitor or not...

    // SetTargetFPS(165);
    
    const int monitorWidth = GetMonitorWidth(monitor);
    const int monitorHeight = GetMonitorHeight(monitor);
    printf("monitor width: %d\nmonitor height: %d\n", monitorWidth, monitorHeight);
    if (imgWidth >= monitorWidth) imgWidth = monitorWidth - 40;
    if (imgHeight >= monitorHeight) imgHeight = monitorHeight - 40;
    SetWindowSize(imgWidth, imgHeight);

    // Clear buffer before drawing
    BeginTextureMode(target);
        ClearBackground(RAYWHITE);
    EndTextureMode();

    // Main loop
    while (!WindowShouldClose()) {
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

        // Backspace event - clear all drawing
        if (IsKeyPressed(KEY_BACKSPACE)) {
            BeginTextureMode(paintingTarget);
                DrawTexture(tex, 0, 0, WHITE);
            EndTextureMode();
        }

        // Brush size
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
            DrawCircle(GetMouseX(), GetMouseY(), brushSize, colourSelected);
        }

        EndDrawing();
    }

    // De-Initialization
    if (saveOnExitFlag) save_image(target, paintingTarget, outputFilepath);
    if (clipboardOnExitFlag) copy_to_clipboard(target, paintingTarget);
    UnloadTexture(tex); // texture unloading
    UnloadRenderTexture(target);
    UnloadRenderTexture(paintingTarget);
    CloseWindow();
    return 0;
}
