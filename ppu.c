/*
 * PPU.c
 *
 *  Created on: Jun 5, 2009
 *  Started again Sept 8, 2009
 *      Author: blanham
 */

/*TODO: REWRITE! */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "B6502.h"
#include "main.h"
#include "PPU.h"



struct	tPPU	ppu;

//not happy with how ppu.OAM_2 is working
BYTE OAM3[8][3];


BYTE chr_buf[0x200][8][8];
BYTE pal_buf[0x2000];

//loopy regs
WORD V, T = 0;
BYTE lpyX;

BYTE spr_buf[0x100];

GLfloat vertices[][3]={
	{-1.0,-1.0,1.0},
	{-1.0,1.0,1.0},
	{1.0,1.0,1.0},
	{1.0,-1.0,1.0},
	{-1.0,-1.0,-1.0},
	{-1.0,1.0,-1.0},
	{1.0,1.0,-1.0},
	{1.0,-1.0,-1.0}
};
unsigned char sprpointer;

void sprrun(void);

GLbyte nes_pal[64][4] = {
	{0x75, 0x75, 0x75, 0},
	{0x21, 0x1b, 0x8f, 0},
	{0x00, 0x00, 0xab, 0},
	{0x47, 0x00, 0x9f, 0},
	{0x8f, 0x00, 0x77, 0},
	{0xab, 0x00, 0x13, 0},
	{0xa7, 0x00, 0x00, 0},
	{0x7f, 0x0b, 0x00, 0},
	{0x43, 0x2f, 0x00, 0},
	{0x00, 0x47, 0x00, 0},
	{0x00, 0x51, 0x00, 0},
	{0x00, 0x3f, 0x17, 0},
	{0x1b, 0x3f, 0x5f, 0},
	{0x00, 0x00, 0x00, 0},
	{0x00, 0x00, 0x00, 0},
	{0x00, 0x00, 0x00, 0},
	{0xbc, 0xbc, 0xbc, 0},
	{0x00, 0x73, 0xef, 0},
	{0x23, 0x3b, 0xef, 0},
	{0x83, 0x00, 0xf3, 0},
	{0xbf, 0x00, 0xbf, 0},
	{0xe7, 0x00, 0x5b, 0},
	{0xdb, 0x2b, 0x00, 0},
	{0xcb, 0x4f, 0x0f, 0},
	{0x8b, 0x73, 0x00, 0},
	{0x00, 0x97, 0x00, 0},
	{0x00, 0xab, 0x00, 0},
	{0x00, 0x93, 0x3b, 0},
	{0x00, 0x83, 0x8b, 0},
	{0x00, 0x00, 0x00, 0},
	{0x00, 0x00, 0x00, 0},
	{0x00, 0x00, 0x00, 0},
	{0xff, 0xff, 0xff, 0},
	{0x3f, 0xbf, 0xff, 0},
	{0x5f, 0x97, 0xff, 0},
	{0xa7, 0x8b, 0xfd, 0},
	{0xf7, 0x7b, 0xff, 0},
	{0xff, 0x77, 0xb7, 0},
	{0xff, 0x77, 0x63, 0},
	{0xff, 0x9b, 0x3b, 0},
	{0xf3, 0xbf, 0x3f, 0},
	{0x83, 0xd3, 0x13, 0},
	{0x4f, 0xdf, 0x4b, 0},
	{0x58, 0xf8, 0x98, 0},
	{0x00, 0xeb, 0xdb, 0},
	{0x00, 0x00, 0x00, 0},
	{0x00, 0x00, 0x00, 0},
	{0x00, 0x00, 0x00, 0},
	{0xff, 0xff, 0xff, 0},
	{0xab, 0xe7, 0xff, 0},
	{0xc7, 0xd7, 0xff, 0},
	{0xd7, 0xcb, 0xff, 0},
	{0xff, 0xc7, 0xff, 0},
	{0xff, 0xc7, 0xdb, 0},
	{0xff, 0xbf, 0x83, 0},
	{0xff, 0xdb, 0xab, 0},
	{0xff, 0xe7, 0xa3, 0},
	{0xe3, 0xff, 0xa3, 0},
	{0xab, 0xf3, 0xbf, 0},
	{0xb3, 0xff, 0xcf, 0},
	{0x9f, 0xff, 0xf3, 0},
	{0x00, 0x00, 0x00, 0},
	{0x00, 0x00, 0x00, 0},
	{0x00, 0x00, 0x00, 0}
};

unsigned char pixels[240][256][4];

//cycle for cycle variables
unsigned char attrshifttable[0x400];
unsigned char attrloc[0x400];

WORD attribshift;


const	unsigned char	ReverseCHR[256] =
{
	0x00,0x80,0x40,0xC0,0x20,0xA0,0x60,0xE0,0x10,0x90,0x50,0xD0,0x30,0xB0,0x70,0xF0,
	0x08,0x88,0x48,0xC8,0x28,0xA8,0x68,0xE8,0x18,0x98,0x58,0xD8,0x38,0xB8,0x78,0xF8,
	0x04,0x84,0x44,0xC4,0x24,0xA4,0x64,0xE4,0x14,0x94,0x54,0xD4,0x34,0xB4,0x74,0xF4,
	0x0C,0x8C,0x4C,0xCC,0x2C,0xAC,0x6C,0xEC,0x1C,0x9C,0x5C,0xDC,0x3C,0xBC,0x7C,0xFC,
	0x02,0x82,0x42,0xC2,0x22,0xA2,0x62,0xE2,0x12,0x92,0x52,0xD2,0x32,0xB2,0x72,0xF2,
	0x0A,0x8A,0x4A,0xCA,0x2A,0xAA,0x6A,0xEA,0x1A,0x9A,0x5A,0xDA,0x3A,0xBA,0x7A,0xFA,
	0x06,0x86,0x46,0xC6,0x26,0xA6,0x66,0xE6,0x16,0x96,0x56,0xD6,0x36,0xB6,0x76,0xF6,
	0x0E,0x8E,0x4E,0xCE,0x2E,0xAE,0x6E,0xEE,0x1E,0x9E,0x5E,0xDE,0x3E,0xBE,0x7E,0xFE,
	0x01,0x81,0x41,0xC1,0x21,0xA1,0x61,0xE1,0x11,0x91,0x51,0xD1,0x31,0xB1,0x71,0xF1,
	0x09,0x89,0x49,0xC9,0x29,0xA9,0x69,0xE9,0x19,0x99,0x59,0xD9,0x39,0xB9,0x79,0xF9,
	0x05,0x85,0x45,0xC5,0x25,0xA5,0x65,0xE5,0x15,0x95,0x55,0xD5,0x35,0xB5,0x75,0xF5,
	0x0D,0x8D,0x4D,0xCD,0x2D,0xAD,0x6D,0xED,0x1D,0x9D,0x5D,0xDD,0x3D,0xBD,0x7D,0xFD,
	0x03,0x83,0x43,0xC3,0x23,0xA3,0x63,0xE3,0x13,0x93,0x53,0xD3,0x33,0xB3,0x73,0xF3,
	0x0B,0x8B,0x4B,0xCB,0x2B,0xAB,0x6B,0xEB,0x1B,0x9B,0x5B,0xDB,0x3B,0xBB,0x7B,0xFB,
	0x07,0x87,0x47,0xC7,0x27,0xA7,0x67,0xE7,0x17,0x97,0x57,0xD7,0x37,0xB7,0x77,0xF7,
	0x0F,0x8F,0x4F,0xCF,0x2F,0xAF,0x6F,0xEF,0x1F,0x9F,0x5F,0xDF,0x3F,0xBF,0x7F,0xFF
};


unsigned char sprcount[240];


const	unsigned long	CHRLoBit[16] =
{
	0x00000000,0x00000001,0x00000100,0x00000101,0x00010000,0x00010001,0x00010100,0x00010101,
	0x01000000,0x01000001,0x01000100,0x01000101,0x01010000,0x01010001,0x01010100,0x01010101
};
const	unsigned long	CHRHiBit[16] =
{
	0x00000000,0x00000002,0x00000200,0x00000202,0x00020000,0x00020002,0x00020200,0x00020202,
	0x02000000,0x02000002,0x02000200,0x02000202,0x02020000,0x02020002,0x02020200,0x02020202
};

const	unsigned char	attrloc1[256] =
{
	0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
	0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
	0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
	0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
	0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
	0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
	0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,
	0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,
	0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,
	0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,
	0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,
	0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,
	0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,
	0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,
	0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
	0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F
};
const	unsigned long	AttribBits[4] =
{
	0x00000000,0x04040404,0x08080808,0x0C0C0C0C
};
const	unsigned char	AttribShift[128] =
{
	0,0,2,2,0,0,2,2,0,0,2,2,0,0,2,2,0,0,2,2,0,0,2,2,0,0,2,2,0,0,2,2,0,0,2,2,0,0,2,2,0,0,2,2,0,0,2,2,0,0,2,2,0,0,2,2,0,0,2,2,0,0,2,2,
	4,4,6,6,4,4,6,6,4,4,6,6,4,4,6,6,4,4,6,6,4,4,6,6,4,4,6,6,4,4,6,6,4,4,6,6,4,4,6,6,4,4,6,6,4,4,6,6,4,4,6,6,4,4,6,6,4,4,6,6,4,4,6,6
};



static const unsigned char attrlut[] =
{
	0x03, 0x07, 0x0B, 0x0F
};


void init_chr_buffer(){
	int i;
	int j;
	int k;
	BYTE pixel;
	unsigned char layerA;
	unsigned char layerB;
	for (i=0;i<256;i++){

			for (j=0;j<8;j++){
					k = 0;
					int l = 7;
					while (k<8)
						{

							layerA = ((CHR[(i*0x10)+j] >> k) & 1);
							layerB = ((CHR[(i*0x10)+0x8+j] >> k) & 1);



							if (layerA & layerB)
								{
									pixel = 0x3;
								}
							else  if (layerA)
								{
									pixel = 0x1;
								}
							else if (layerB)
								{
									pixel = 0x2;
								}
							else
								{
									pixel = 0x00;
								}

							chr_buf[i][j][l] = pixel;
							l--;
							k++;

						}
				}
		}

}

GLvoid *colors;

void initPPU()
{
	RAM[0x2002] = 0x10;
	ppu.scanline = -1;
	ppu.rendering = 0;
	ppu.endcycle = 341;
	ppu.cycle = 0;
	//builds shift tables
	int i;
	for (i = 0; i != 0x400; ++i)
		{
			attrshifttable[i] = ((i >> 4) & 0x04) | (i & 0x02);
			attrloc[i] =   (((i >> 4) & 0x38) | (i >> 2))  &  7;
		}

		
	init_chr_buffer();
}


/* derp, move this */
void nmi()
{
#ifdef PRNTSTAT
	printf("\n\nNMI!\n\n");
#endif
	RAM[0x0100+STACK--]= (PC >> 8);
	RAM[0x0100+STACK--]= (PC & 0xff);
	RAM[0x0100+STACK--]= P;
	PC = RAM[0xfffa] + (RAM[0xfffb]<<8);
	RAM[0x2000] |= 0x80;
	NMI=0;
}

inline unsigned char rdVRAM(WORD addr){

	BYTE value= 0;

	if (addr>=0x2000&&addr<0x3F00){

			value = rdNT(addr);

		}else if (addr>=0x3F00){

			if (addr == 0x4 || addr == 0x8 || addr == 0xc) addr = 0x0;
			value = PAL[addr&0xFF];

		}else    if (addr<0x2000){

			value = rdCHRRAM(addr);

		}


	return value;


}

inline void wrVRAM(WORD addr, BYTE value){
	addr &= 0x3FFF;
	if (addr>=0x2000 && addr<0x3F00){
			//printf("ADDR:%.4X\n",addr);
			rdNT(addr) = value;
		}else if (addr>=0x3F00)
		{
			if (addr==0x3f10  ||addr == 0x3f14 || addr == 0x3f18 || addr == 0x3f1c) addr -= 0x10;
			PAL[addr&0x3F] = value;
		}else     if (addr<0x2000){
			rdCHRRAM(addr)=value;
		}
}







void RunPPU(int cycles)
{
				




		



}

