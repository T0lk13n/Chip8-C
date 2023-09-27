#pragma once

#define screenW 64
#define screenH 32 
#define MEM_SIZE 4096
#define MEM_PROGRAM_START 0x200
#define FONTS_ADDR 0
#define FONTS_SIZE 5*16


// *************************************************
//	Internal Font data	8x5 pixeles
// *************************************************
unsigned char fonts [16][5] =
{
	{0xF0, 0x90, 0x90, 0x90, 0xF0},// 0
	{0x20, 0x60, 0x20, 0x20, 0x70},// 1
	{0xF0, 0x10, 0xF0, 0x80, 0xF0},// 2
	{0xF0, 0x10, 0xF0, 0x10, 0xF0},// 3
	{0x90, 0x90, 0xF0, 0x10, 0x10},// 4
	{0xF0, 0x80, 0xF0, 0x10, 0xF0},// 5
	{0xF0, 0x80, 0xF0, 0x90, 0xF0},// 6
	{0xF0, 0x10, 0x20, 0x40, 0x40},// 7
	{0xF0, 0x90, 0xF0, 0x90, 0xF0},// 8
	{0xF0, 0x90, 0xF0, 0x10, 0xF0},// 9
	{0xF0, 0x90, 0xF0, 0x90, 0x90},// A
	{0xE0, 0x90, 0xE0, 0x90, 0xE0},// B
	{0xF0, 0x80, 0x80, 0x80, 0xF0},// C
	{0xE0, 0x90, 0x90, 0x90, 0xE0},// D
	{0xF0, 0x80, 0xF0, 0x80, 0xF0},// E
	{0xF0, 0x80, 0xF0, 0x80, 0x80} // F
};


struct chip8_t
{
	unsigned char  mem[MEM_SIZE];	//memoria
	unsigned short pc;				//program counter
	unsigned char  v[16];			//registros generales
	unsigned short I;				//registro I
	unsigned char  delayTimer;		//temporizador
	unsigned char  soundTimer;		//temporizador audio
	unsigned short stack[16];		//stack
	unsigned short sp;				//stack pointer
};

void initChip8(struct chip8_t *chip8);
void loadRom(const char* filename);