#include "chip8.h"


int main(void)
{
	SetTraceLogLevel(LOG_NONE);

	// Initialization
	//--------------------------------------------------------------------------------------
	const int screenWidth  = screenW * fontSize;
	const int screenHeight = screenH * fontSize;
	InitWindow(screenWidth, screenHeight, "Chip 8 emulator");
	ClearBackground(BLACK);
	SetTargetFPS(60);               // Set our game to run at 60 frames-per-second

	struct chip8_t*  chip8 = (struct chip8_t*) malloc(sizeof(struct  chip8_t));
	struct opcode_t* opcode= (struct opcode_t*)malloc(sizeof(struct opcode_t));

	initChip8(chip8);

	// https://github.com/Timendus/chip8-test-suite
	//loadRom(chip8, "tests/1-chip8-logo.ch8");
	//loadRom(chip8, "tests/2-ibm-logo.ch8");
	//loadRom(chip8, "tests/3-corax+.ch8");
	//loadRom(chip8, "tests/4-flags.ch8");
	//loadRom(chip8, "tests/5-quirks.ch8");
	//loadRom(chip8, "tests/6-keypad.ch8");
	
	//loadRom(chip8, "tests/test_opcode.ch8");
	//loadRom(chip8, "tests/sctest.ch8");
	//loadRom(chip8, "tests/c8_test.ch8");
	//loadRom(chip8, "chip8-roms/games/airplane.ch8");
	loadRom(chip8, "chip8-roms/games/cave.ch8");
	//loadRom(chip8, "chip8-roms/games/pong (alt).ch8");
	//loadRom(chip8, "chip8-roms/programs/ibm logo.ch8");

	//--------------------------------------------------------------------------------------
	// Main game loop
	while (!WindowShouldClose())    // Detect window close button or ESC key
	{
		// Update
		if (IsFileDropped())
		{
			initChip8(chip8);
			FilePathList dropedFiles = LoadDroppedFiles();
			loadRom(chip8, dropedFiles.paths[0]);
			UnloadDroppedFiles(dropedFiles);
		}

		getInput();

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

		// Volcamos buffer
		for (int y = 0; y < screenH; y++)
		{
			for (int x = 0; x < screenW; x++)
			{
				if (buffer[x][y] == 1)
					//DrawPixel(x, y, WHITE);
					DrawRectangle((x * fontSize), (y * fontSize), fontSize, fontSize, WHITE);
				else
					DrawRectangle((x * fontSize), (y * fontSize), fontSize, fontSize, BLACK);
			}
		}


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
	ClearBackground(BLACK);

	// Clear buffer
	for (int y = 0; y < screenH; y++)
	{
		for (int x = 0; x < screenW; x++)
		{
			buffer[x][y] = 0;
			DrawRectangle((x * fontSize), (y * fontSize), fontSize, fontSize, BLACK);
		}
	}
	
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

/*
				GET OPCODE
*/
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


/*
				DECODE OPCODE
*/
void decodeOpcode(struct chip8_t* chip8, struct opcode_t *opcode)
{
	switch (opcode->instruction)
	{
		case 0:
			// 00E0 : Clear screen
			// 00EE - RET
			if (opcode->nn == 0xE0)
				//ClearBackground(BLACK);
				for (int y = 0; y < screenH; y++)
				{
					for (int x = 0; x < screenW; x++)
					{
						buffer[x][y] = 0;					
					}
				}
			else if (opcode->nn == 0xEE)
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
			break;

		case 8:
			switch (opcode->n)
			{
				case 0:
					// Set Vx = Vy.
					// Stores the value of register Vy in register Vx.
					chip8->v[opcode->x] = chip8->v[opcode->y];
					break;

				case 1:
					// Set Vx = Vx OR Vy.
					// Performs a bitwise OR on the values of Vx and Vy, 
					// then stores the result in Vx.A bitwise OR compares the corrseponding bits from two values, 
					// and if either bit is 1, then the same bit in the result is also 1. Otherwise, it is 0.
					chip8->v[opcode->x] = chip8->v[opcode->x] | chip8->v[opcode->y];
					break;

				case 2:
					// Set Vx = Vx AND Vy.
					// Performs a bitwise AND on the values of Vx and Vy, then stores the result in Vx.
					// A bitwise AND compares the corrseponding bits from two values, and if both bits are 1, 
					// then the same bit in the result is also 1. Otherwise, it is 0.
					chip8->v[opcode->x] = chip8->v[opcode->x] & chip8->v[opcode->y];
					break;

				case 3:
					// Set Vx = Vx XOR Vy.
					// Performs a bitwise exclusive OR on the values of Vx and Vy, then stores the result in Vx.
					// An exclusive OR compares the corrseponding bits from two values, and if the bits are not both the same, 
					// then the corresponding bit in the result is set to 1. Otherwise, it is 0.
					chip8->v[opcode->x] = chip8->v[opcode->x] ^ chip8->v[opcode->y];
					break;
				
				case 4:
					// Set Vx = Vx + Vy, set VF = carry.
					// The values of Vx and Vy are added together.If the result is greater than 8 bits(i.e., > 255, ) VF is set to 1, otherwise 0. 
					// Only the lowest 8 bits of the result are kept, and stored in Vx.
					{
						int overflow = chip8->v[opcode->x] + chip8->v[opcode->y];
						if (overflow > 255)
							chip8->v[15] = 1;
						else
							chip8->v[15] = 0;
						chip8->v[opcode->x] = overflow & 0xff;
						break;
					}

				case 5:
					// Set Vx = Vx - Vy, set VF = NOT borrow.
					// If Vx > Vy, then VF is set to 1, otherwise 0. Then Vy is subtracted from Vx, and the results stored in Vx.
					if (chip8->v[opcode->x] > chip8->v[opcode->y])
						chip8->v[15] = 1;
					else
						chip8->v[15] = 0;
					chip8->v[opcode->x] -= chip8->v[opcode->y] & 0xff;
					break;

				case 6:
					// Set VX equal to VX bitshifted right 1. 
					// VF is set to the least significant bit of VX prior to the shift
					//chip8->v[opcode->x] = chip8->v[opcode->y];

					if ((chip8->v[opcode->x] & 1) == 1)
						chip8->v[15] = 1;
					else
						chip8->v[15] = 0;
					chip8->v[opcode->x] = chip8->v[opcode->x] >> 1;
					break;

				case 7:
					// Set Vx = Vy - Vx, set VF = NOT borrow.
					// If Vy > Vx, then VF is set to 1, otherwise 0. Then Vx is subtracted from Vy, and the results stored in Vx.
					if (chip8->v[opcode->y] > chip8->v[opcode->x])
						chip8->v[15] = 1;
					else
						chip8->v[15] = 0;
					chip8->v[opcode->x] = chip8->v[opcode->y] - chip8->v[opcode->x];
					break;

				case 0xe:  //E
					// Set VX equal to VX bitshifted left 1. 
					// VF is set to the most significant bit of VX prior to the shift
					//chip8->v[opcode->x] = chip8->v[opcode->y];

					if ((chip8->v[opcode->x] & 0b10000000) == 0b10000000)
						chip8->v[15] = 1;
					else
						chip8->v[15] = 0;
					chip8->v[opcode->x] = chip8->v[opcode->y] << 1;
					break;
			}
	
		case 9:
			// Skip next instruction if Vx != Vy.
			// The values of Vx and Vy are compared, and if they are not equal, the program counter is increased by 2.
			if (chip8->v[opcode->x] != chip8->v[opcode->y])
				PC += 2;
			break;

		case 0xa: //A
			// Set I = nnn.
			// The value of register I is set to nnn.
			chip8->I = opcode->nnn;
			break;

		case 0xb: //B
			// Jump to location nnn + V0.
			// The program counter is set to nnn plus the value of V0.
			PC = opcode->nnn + chip8->v[0];
			break;
	
		case 0xc: //C
			//Set Vx = random byte AND nn.
			// The interpreter generates a random number from 0 to 255, which is then ANDed with the value nn.The results are stored in Vx.
			// See instruction 8xy2 for more information on AND.
			srand((unsigned int)time(NULL));
			chip8->v[opcode->x] = (rand() % 0xff) & (opcode->nn);		
			break;

		case 0xd: //D
			// Display n - byte sprite starting at memory location I at(Vx, Vy), set VF = collision.
			// The interpreter reads n bytes from memory, starting at the address stored in I.These bytes are then displayed as sprites on screen at coordinates(Vx, Vy).
			// Sprites are XORed onto the existing screen.If this causes any pixels to be erased, VF is set to 1, otherwise it is set to 0. 
			// If the sprite is positioned so part of it is outside the coordinates of the display, it wraps around to the opposite side of the screen.
			// See instruction 8xy3 for more information on XOR, and section 2.4, Display, for more information on the Chip - 8 screen and sprites.
			{
				chip8->v[15] = 0;
				int mask = 128; //1000 0000 en binario
				//int x = chip8->v[opcode->x] * fontSize;
				//int y = chip8->v[opcode->y] * fontSize;
				int px = chip8->v[opcode->x];
				int py = chip8->v[opcode->y];

				for (int y = 0; y < opcode->n; y++)
				{
					for (int x = 0; x < 8; x++)
					{
						unsigned char mem = ((chip8->mem[chip8->I + y]) & mask) >> (7 - x);
						unsigned char xor = mem ^ (buffer[px + x][py + y]);
						
						if((mem == 1) & (buffer[px + x][py + y] == 1))
								chip8->v[15] = 1;
		
						buffer[px + x][py + y] = xor;
						mask = mask >> 1;				
					}
					mask = 128;
				}
				break;
			}
		
		case 0xe: //E
			//printf("Key: %d\n opcode:%d\n", keyPressed, chip8->v[opcode->x]);
			if (opcode->nn == 0x9e)
			{
				// Skip next instruction if key with the value of Vx is pressed.
				// Checks the keyboard, and if the key corresponding to the value of Vx is currently in the down position, PC is increased by 2.
				if (IsKeyDown(keys[chip8->v[opcode->x]]))
					PC += 2;
			}
			else  if(opcode->nn == 0xa1) //(0xa1)
			{
				// Skip next instruction if key with the value of Vx is not pressed.
				// Checks the keyboard, and if the key corresponding to the value of Vx is currently in the up position, PC is increased by 2.
				if(!IsKeyDown(keys[chip8->v[opcode->x]]) )
					PC += 2;
			}
			break;

		case 0xf: //F
			switch (opcode->nn)
			{
				case 07:
					//Set Vx = delay timer value.	
					//The value of DT is placed into Vx.
					chip8->v[opcode->x] = chip8->delayTimer;
					break;

				case 0x0a: //0A
					// Wait for a key press, store the value of the key in Vx.
					// All execution stops until a key is pressed, then the value of that key is stored in Vx.
					break;

				case 0x15: //15
					// Set delay timer = Vx.
					// DT is set equal to the value of Vx.
					chip8->delayTimer = chip8->v[opcode->x];
					break;

				case 0x18: //18
					// Set sound timer = Vx.
					// ST is set equal to the value of Vx.
					chip8->soundTimer = chip8->v[opcode->x];
					break;

				case 0x1e: //1E
					// Set I = I + Vx.
					// The values of I and Vx are added, and the results are stored in I.
					chip8->I += chip8->v[opcode->x];
					break;

				case 0x29: //29
					// Set I = location of sprite for digit Vx.
					// The value of I is set to the location for the hexadecimal sprite corresponding to the value of Vx.
					// See section 2.4, Display, for more information on the Chip - 8 hexadecimal font.
					chip8->I = FONTS_ADDR + (chip8->v[opcode->x]*5); // &0x0f;
					break;

				case 0x33: //33
					// Store BCD representation of Vx in memory locations I, I + 1, and I + 2.
					// The interpreter takes the decimal value of Vx,
					// and places the hundreds digit in memory at location in I, the tens digit at location I + 1, and the ones digit at location I + 2.
					{
					unsigned char BCD = chip8->v[opcode->x];
					unsigned short location = chip8->I;
					chip8->mem[location] = BCD / 100;
					chip8->mem[location + 1] = (BCD % 100) /10;
					chip8->mem[location + 2] = BCD % 10;
					}
					break;

				case 0x55: //55
					// Store registers V0 through Vx in memory starting at location I.
					// The interpreter copies the values of registers V0 through Vx into memory, starting at the address in I.
					for (int i = 0; i <= opcode->x; i++)
						chip8->mem[chip8->I+i] = chip8->v[i];
					break;
				
				case 0x65: //65
					// Read registers V0 through Vx from memory starting at location I.
					// The interpreter reads values from memory starting at location I into registers V0 through Vx.
					for (int i = 0; i <= opcode->x; i++)
						chip8->v[i] = chip8->mem[chip8->I + i];
					break;
			}
			break;
	}
}


void getInput()
{
	//ZOOM
	if (IsKeyPressed(KEY_DOWN))
	{
		fontSize--;
		if (fontSize < 1) fontSize = 1;
		SetWindowSize(screenW * fontSize, screenH * fontSize);
		// puts("z");
	}
	else if (IsKeyPressed(KEY_UP))
	{
		fontSize++;
		if (fontSize > 10) fontSize = 10;
		SetWindowSize(screenW * fontSize, screenH * fontSize);
		//puts("x");
	}
}
