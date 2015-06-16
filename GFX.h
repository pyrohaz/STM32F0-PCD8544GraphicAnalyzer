#ifndef GFX_H
#define GFX_H

#include <PCD8544F0.h>
#include <math.h>
//#include <stdlib.h>

/*
 * GFX.h
 * Author: Harris Shallcross
 * Year: 2014-ish
 *
 * General GFX library, uses a framebuffer (xbyte) for all functions.
 *
 * This code is provided AS IS and no warranty is included!
 */

#define LetterSpace 0

typedef enum PTypes{
	PixNorm = 0,
	PixInv = !PixNorm,
} PixT;

void DispMode(uint8_t);

uint8_t WritePix(int16_t, int16_t, PixT);
uint8_t SetPix(uint8_t, uint8_t);
uint8_t ClrPix(uint8_t, uint8_t);

uint8_t Circle(uint8_t, uint8_t, uint8_t, PixT);
uint8_t Semicircle(uint8_t, uint8_t, uint8_t, uint8_t, PixT);

uint8_t Square(uint8_t, uint8_t, uint8_t, uint8_t, PixT);
uint8_t LineP(uint8_t, uint8_t, uint8_t, int16_t, PixT);
uint8_t LineL(uint8_t, uint8_t, uint8_t, uint8_t, PixT);

int PChar(uint16_t, int16_t, int16_t, uint8_t, PixT);
int PStr(const char*, int16_t, int16_t, uint8_t, PixT);
int PNum(int32_t, int16_t, int16_t, int8_t, uint8_t, PixT);
int PNumF(float, int16_t, int16_t, uint8_t, uint8_t, PixT);

int32_t FPow(int32_t, int32_t);
int32_t Abs(int32_t);
uint32_t Strlen(const char *);
void DrawCol(uint8_t Col, uint8_t Val);

uint8_t BatteryIcon(uint8_t, uint8_t, uint8_t);

extern uint8_t GBuf[GBufS];

#endif
