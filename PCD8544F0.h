#ifndef PCD8544F0_H
#define PCD8544F0_H

#include <stm32f0xx_gpio.h>
#include <stm32f0xx_rcc.h>
#include <stm32f0xx_spi.h>

typedef enum WModesS{
	Dat,
	Reg,
} WModes;

//Main functions
void PCD8544_InitSetup();
void SB(uint8_t, uint8_t, uint8_t);
void PScrn(void);
void ClrBuf(void);
void ScreenOn(void);
void ScreenOff(void);
void NormalMode(void);
void InvertMode(void);
uint8_t BacklightIO(uint8_t);

void Delay(uint32_t);

#define NOP 0
#define FuncSetE 	0b00100001
#define FuncSetSV 	0b00100000
#define FuncSetSH 	0b00100010
#define SetVOP 		0b10000000
#define DispBlnk 	0b00001000
#define DispAll 	0b00001001
#define DispNorm	0b00001100
#define InvDisp 	0b00001101
#define BiasSet 	0b00010000
#define TempSet 	0b00000100
#define SetYAdd 	0b01000000
#define SetXAdd 	0b10000000

#define Clk GPIO_Pin_5
#define DIn GPIO_Pin_7
#define DC GPIO_Pin_3
#define CE GPIO_Pin_2
#define RS GPIO_Pin_1
#define BL1 GPIO_Pin_9
#define BL2 GPIO_Pin_10
#define VCC GPIO_Pin_4

#define ClkPS GPIO_PinSource5
#define DInPS GPIO_PinSource7

#define IOGPIO GPIOA
#define VCCGPIO GPIOB

#define XPix 84
#define YPix 48
#define GBufS ((XPix*YPix)/8)

extern uint8_t Mode;
extern uint8_t GBuf[GBufS];

#endif
