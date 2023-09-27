#include <stdlib.h>
#include <stdio.h>
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

	struct chip8_t*  chip8 = (struct chip8_t*) malloc(sizeof(struct  chip8_t));
	struct opcode_t* opcode= (struct opcode_t*)malloc(sizeof(struct opcode_t));

	initChip8(chip8);
	loadRom(chip8, "tests/test_opcode.ch8");




	//--------------------------------------------------------------------------------------
	// Main game loop
	while (!WindowShouldClose())    // Detect window close button or ESC key
	{
		// Update
		if (IsFileDropped())
		{
			FilePathList dropedFiles = LoadDroppedFiles();
			loadRom(chip8, dropedFiles.paths[0]);
			UnloadDroppedFiles(dropedFiles);
		}

		int numOpcodes = 0;
		while (numOpcodes < OPCODESPERFRAME)
		{
			getOpcode(chip8, opcode);
			decodeOpcode(chip8, opcode);
			numOpcodes++;
		}

		// temporizadores
		chip8->soundTimer--;
		chip8->delayTimer--;
		if (chip8->soundTimer < 0)
			chip8->soundTimer = 0xff;
		if (chip8->delayTimer < 0)
			chip8->delayTimer = 0xff;


		// Draw
		//----------------------------------------------------------------------------------
		BeginDrawing();

		//ClearBackground(BLACK);


		EndDrawing();
		//----------------------------------------------------------------------------------
	}

	// De-Initialization
	if (chip8) free(chip8);
	if (opcode) free(opcode);
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
	SP = 0;
	chip8->delayTimer = 0xff;
	chip8->soundTimer = 0xff;
	// Program counter a la direccion donde empiezan los programas
	PC = MEM_PROGRAM_START;

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



void loadRom(struct chip8_t *chip8,  const char* filename)
{
	//carga la rom en memoria a partir de MEM_PROGRAM_START
	if(FileExists(filename))
	{
		int fileSize = GetFileLength(filename);
		// chip8->mem = LoadFileData(filename, fileSize);
		FILE* file = fopen(filename, "rb");
		
		//errno_t  err = fopen_s(&file, filename, "Rb");
		for (int i = 0; i < fileSize; i++)
		{
			chip8->mem[MEM_PROGRAM_START + i] = fgetc(file);
		}
	}
}


void getOpcode(struct chip8_t* chip8, struct opcode_t *opcode)
{
	// Se descompone asi
	// Primer nibble Instruccion

	// X : The second nibble. Used to look up one of the 16 registers (VX) from V0 through VF.
	// Y : The third nibble.Also used to look up one of the 16 registers(VY) from V0 through VF.
	// N : The fourth nibble.A 4 - bit number.
	// NN : The second byte(third and fourth nibbles).An 8 - bit immediate number.
	// NNN : The second, third and fourth nibbles.A 12 - bit immediate memory address.

	opcode->instruction = CURRENTOPCODE;

	PC += 2;
	//evitamos leer mas allá de la memoria
	if (PC > MEM_SIZE)
		PC = 0;

	// Descomponemos la instruccion para poder despues operar con sus datos
	opcode->x  = (opcode->instruction & 0x0f00) >> 8;
	opcode->y  = (opcode->instruction & 0x00f0) >> 4;
	opcode->n  = opcode->instruction & 0x000f;
	opcode->nn = opcode->instruction & 0x00ff;
	opcode->nnn = opcode-> instruction & 0x0fff;
	opcode->instruction = (opcode->instruction & 0xf000) >> 12;
}


void decodeOpcode(struct chip8_t* chip8, struct opcode_t *opcode)
{
	/*  PARA EMPEZAR

		00E0 (clear screen)
		1NNN(jump)
		6XNN(set register VX)
		7XNN(add value to register VX)
		ANNN(set index register I)
		DXYN(display / draw)
	*/
	switch (opcode->instruction)
	{
		case 0:
			// 00E0 : Clear screen
			// 00EE - RET
			if (opcode->nn == 0xE0)
				ClearBackground(BLACK);
			else
				// Return from a subroutine.
				// The interpreter sets the program counter to the address at the top of the stack, then subtracts 1 from the stack pointer.
				PC = chip8->stack[SP];
				SP--;
			break;

		case 1:
			//1NNN : Jump
			PC = opcode->nnn;
			break;

		case 2:
			// CALL addr
			// Call subroutine at nnn.
			// The interpreter increments the stack pointer, then puts the current PC on the top of the stack.The PC is then set to nnn.
			SP++;
			chip8->stack[SP] = PC;
			PC = opcode->nnn;
			break;

		case 3:
			// Skip next instruction if Vx = nn.
			// The interpreter compares register Vx to nn, and if they are equal, increments the program counter by 2.
			if (chip8->v[opcode->x] == opcode->nn)
				PC += 2;
			break;

		case 4:
			// Skip next instruction if Vx != nn.
			// The interpreter compares register Vx to nn, and if they are not equal, increments the program counter by 2.
			if (chip8->v[opcode->x] != opcode->nn)
				PC += 2;
			break;
		
		case 5:
			// Skip next instruction if Vx = Vy.
			// The interpreter compares register Vx to register Vy, and if they are equal, increments the program counter by 2.
			if (chip8->v[opcode->x] == chip8->v[opcode->y])
				PC += 2;
			break;

		case 6:
			// Set Vx = nn.
			// The interpreter puts the value nn into register Vx.
			chip8->v[opcode->x] = opcode->nn;
			break;

		case 7:
			// Set Vx = Vx + nn.
			// Adds the value nn to the value of register Vx, then stores the result in Vx.
			chip8->v[opcode->x] += opcode->nn;
			break:

		case 8:
	}
}