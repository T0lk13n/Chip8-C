#include <stdlib.h>
#include "raylib.h"
#include "chip8.h"




int main(void)
{
    SetTraceLogLevel(LOG_NONE);

    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth  = screenW *10;
    const int screenHeight = screenH *10;
    InitWindow(screenWidth, screenHeight, "Chip 8 emulator");
    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second

    struct chip8_t* chip8 = (struct chip8_t*)malloc(sizeof(struct chip8_t));

    initChip8(chip8);

    //--------------------------------------------------------------------------------------
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

        ClearBackground(BLACK);


        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    if (chip8) free(chip8);
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}




void initChip8(struct chip8_t* chip8)
{
    // Clear Ram
    for (unsigned int i = 0; i < MEM_SIZE; i++)
        chip8->mem[i] = 0;
    // Clear Registers and stack
    for (unsigned int i = 0; i < 16; i++)
    {
        chip8->v[i] = 0;
        chip8->stack[i] = 0;
    }
    // More clean
    chip8->I = 0;
    chip8->sp = 0;
    chip8->delayTimer = 0xff;
    chip8->soundTimer = 0xff;

    // Copiamos fonts a memoria
    int memIndex = 0;
    for (int i = 0; i < 16; i++)
    {
        for (int j = 0; j < 5; j++)
        {
            chip8->mem[FONTS_ADDR + memIndex] = fonts[i][j];
            memIndex++;
        }
            
    }


}

void loadRom(const char* filename)
{
    //carga la rom en memoria a partir de MEM_PROGRAM_START
}