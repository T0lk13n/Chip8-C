#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <time.h>    //for random seed
#include "raylib.h"

#define screenW 64
#define screenH 32 
#define MEM_SIZE 4096
#define MEM_PROGRAM_START 0x200
#define FONTS_ADDR 0x50
#define FONTS_SIZE 5*16
#define OPCODESPERFRAME 10
#define PC chip8->pc
#define SP chip8->sp
#define CURRENTOPCODE (chip8->mem[PC] << 8 | chip8->mem[PC+1])
#define VX chip8->v[opcode->x]
#define VY chip8->v[opcode->y]
#define VF chip8->v[15]

int fontSize = 12;
// buffer pintado
unsigned char buffer[64][32];

const unsigned int keys[]=
{
	KEY_X,
	KEY_ONE,	//1
	KEY_TWO,	//2
	KEY_THREE,	//3

	KEY_Q,
	KEY_W,
	KEY_E,
	KEY_A,

	KEY_S,
	KEY_D,
	KEY_Z,
	KEY_C,

	KEY_FOUR,	//...
	KEY_R,
	KEY_F,
	KEY_V
};


// *************************************************
//	Internal Font data	8x5 pixeles
// *************************************************
const unsigned char fonts [16][5] =
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


// Maquina a 500-600 Hz. Hay que limitar la velocidad a unos 10 opcodes por frame
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

struct opcode_t
{
	unsigned short	instruction;
	unsigned char	x;
	unsigned char	y;
	unsigned char	n;
	unsigned char	nn;
	unsigned short	nnn;
};

void initChip8(struct chip8_t *chip8);
void loadRom(struct chip8_t *chip8, const char* filename);
void getOpcode(struct chip8_t *chip8, struct opcode_t *opcode);
void decodeOpcode(struct chip8_t* chip8, struct opcode_t *opcode);
void getInput();
