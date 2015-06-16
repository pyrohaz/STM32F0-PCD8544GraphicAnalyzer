/*
 * The code for a PCD8544 based graphic analyzer using the inbuilt ADC on the
 * STM32F0 Discovery board
 *
 * Author: Harris Shallcross
 * Year: 16/6/2015
 *
 *
 * Code and example descriptions can be found on my blog at:
 * www.hsel.co.uk
 *
 * ALL OTHER CODE LICENSES STILL APPLY! This license only applies to my code.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stm32f0xx_gpio.h>
#include <stm32f0xx_adc.h>
#include <stm32f0xx_rcc.h>
#include "GFX.h"
#include <math.h>

//Sample rate
#define FS		40000

//Oversampling definition, used for software filtering!
#define Oversampling2x

//Amount of samples to obtain
#define Len		512

//Log2(AmntSamples)
#define L2Len	9

//Data ready flag
volatile uint8_t DataGot = 0;

//Sample storage array
volatile int16_t Samples[Len] = {0};

//Cosine window LUT
const uint16_t CWLen = 128;
const uint8_t CWBD = 16;
const int16_t CosWindow[] = {    0,    20,    79,   177,   315,   491,   705,   958,  1247,  1573,  1934,  2331,  2761,  3224,
		3719,  4244,  4798,  5381,  5990,  6624,  7281,  7960,  8660,  9378, 10113, 10864, 11627, 12402, 13187,
		13979, 14777, 15579, 16383, 17187, 17989, 18787, 19579, 20364, 21139, 21902, 22653, 23388, 24106, 24806,
		25485, 26142, 26776, 27385, 27968, 28522, 29047, 29542, 30005, 30435, 30832, 31193, 31519, 31808, 32061,
		32275, 32451, 32589, 32687, 32746, 32766, 32746, 32687, 32589, 32451, 32275, 32061, 31808, 31519, 31193,
		30832, 30435, 30005, 29542, 29047, 28522, 27968, 27385, 26776, 26142, 25485, 24806, 24106, 23388, 22653,
		21902, 21139, 20364, 19579, 18787, 17989, 17187, 16383, 15579, 14777, 13979, 13187, 12402, 11627, 10864,
		10113,  9378,  8660,  7960,  7281,  6624,  5990,  5381,  4798,  4244,  3719,  3224,  2761,  2331,  1934,
		1573,  1247,   958,   705,   491,   315,   177,    79,    20
};

typedef enum Windows{
	Triangular = 0,
	Hann = 1,
	Rectangular = 2
} Windows;

const uint8_t ColumnFilter = 1;
const uint8_t Decibels = 0;
Windows Window = Hann;

void SysTick_Handler(void){
	//Sample counter
	static uint16_t SCnt = 0;

	//Oversampling counter
	static uint8_t OSCnt = 0;

	//Sample in variable
	static int32_t SmpI;

	//Low passed sample variable
	static int32_t SmpLP = 2048;

#ifndef Oversampling2x
	//Grab latest ADC value, shift data up by 4 to scale from 12bit to 16bit
	SmpI = ADC_GetConversionValue(ADC1)<<4;
#else
	//Grab and filter latest ADC value at ~20kHz
	SmpI += (((int32_t)(ADC_GetConversionValue(ADC1)<<4)-SmpI)*48030)>>16;
#endif

	//Set off next conversion
	ADC_StartOfConversion(ADC1);

	OSCnt++;
#ifdef Oversampling2x
	if(OSCnt == 2){
#else
	if(OSCnt == 1){
#endif
		OSCnt = 0;
		//If data hasn't been got, get data
		if(!DataGot){
			//Low pass ADC data @ 26Hz
			SmpLP += (SmpI-SmpLP)>>8;
			//High pass filter data
			Samples[SCnt] = (SmpI - SmpLP)>>4;

			//Increment sample counter
			SCnt++;

			//Check to see if 'max samples' has been obtained
			if(SCnt == Len){
				//Reset sample counter
				SCnt = 0;

				//Set 'DataGot' flag
				DataGot = 1;
			}
		}
	}
}

/*Integer square root - Obtained from Stack Overflow (14/6/15):
 * http://stackoverflow.com/questions/1100090/looking-for-an-efficient-integer-square-root-algorithm-for-arm-thumb2
 * User: Gutskalk
 */
uint16_t isqrt(uint32_t x)
{
	uint16_t res=0;
	uint16_t add= 0x8000;
	int i;
	for(i=0;i<16;i++)
	{
		uint16_t temp=res | add;
		uint32_t g2=temp*temp;
		if (x>=g2)
		{
			res=temp;
		}
		add>>=1;
	}
	return res;
}

ADC_InitTypeDef A;
GPIO_InitTypeDef G;

/* Function prototype to fix_fft library, obtained from:
 * http://www.jjj.de/fft/fftpage.html (14/6/15)
 */
int fix_fft(short fr[], short fi[], short m, short inverse);

int main(void)
{
	//Initialize clocks
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

	//Configure PA0 as analog input
	G.GPIO_Pin = GPIO_Pin_0;
	G.GPIO_Mode = GPIO_Mode_AN;
	G.GPIO_PuPd = GPIO_PuPd_NOPULL;
	G.GPIO_OType = GPIO_OType_PP;
	G.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOA, &G);

	//Configure ADC
	A.ADC_ContinuousConvMode = DISABLE;
	A.ADC_DataAlign = ADC_DataAlign_Right;
	A.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
	A.ADC_Resolution = ADC_Resolution_12b;
	A.ADC_ScanDirection = ADC_ScanDirection_Upward;
	ADC_Init(ADC1, &A);
	ADC_Cmd(ADC1, ENABLE);

	//Set ADC channel to 0
	ADC_ChannelConfig(ADC1, ADC_Channel_0, ADC_SampleTime_7_5Cycles);

	//Start first conversion (before interrupt is fired!)
	ADC_StartOfConversion(ADC1);

	//Initialize LCD
	PCD8544_InitSetup();

#ifndef Oversampling2x
	//Configure SysTick to interrupt at sample rate
	SysTick_Config(SystemCoreClock/FS);
#else
	//Configure interrupt to sample at 2x FS
	SysTick_Config(SystemCoreClock/(FS*2));
#endif

	//Array to store imaginary variables
	int16_t Imag[Len];

	//Column array to store amplitudes
	uint8_t Col[XPix];
	uint32_t Cnt, Index, IndO, BufSum = 0;

	while(1)
	{
		//Wait for Data
		while(!DataGot);
		//If 'Window' isn't rectangular, apply window
		if(Window == Triangular){
			//Apply a triangular window to the data.
			for(Cnt = 0; Cnt<Len; Cnt++){
				if(Cnt<(Len/2)) Samples[Cnt] = ((int32_t)Samples[Cnt]*Cnt)>>L2Len;
				else Samples[Cnt] = ((int32_t)Samples[Cnt]*((Len/2)-Cnt))>>L2Len;
			}
		}
		else if(Window == Hann){
			//Use the cosine window wavetable to apply a Hann windowing function
			//to the samples
			for(Cnt = 0; Cnt<Len; Cnt++){
				Index = (Cnt*CWLen)>>L2Len;
				Samples[Cnt] = ((int32_t)Samples[Cnt]*(int32_t)CosWindow[Index])>>(CWBD);
			}
		}

		//No imaginary data used for the FFT, therefore, initialize
		//'Imag' array as zeros
		memset(Imag, 0, Len*sizeof(int16_t));

		//Calculate FFT of samples
		fix_fft(Samples, Imag, L2Len, 0);

		IndO = 0;
		BufSum = 0;

		int32_t R, I;

		//Calculate the magnitude
		for(Cnt = 0; Cnt<Len/2; Cnt++){
			//Square the real components
			R = (int32_t)Samples[Cnt]*(int32_t)Samples[Cnt];

			//Square the imaginary components
			I = (int32_t)Imag[Cnt]*(int32_t)Imag[Cnt];

			//Calculate the magnitude and sum into BufSum. As there
			//are more samples than columns, sum the magnitude of
			//the bins. E.g. for 128 samples and 64 pixels on the LCD,
			//each column should be the sum of 2 bins (128/64). If there
			//are 512 samples and 64 pixels on the LCD, each column should
			//be the sum of 8 bins (512/64)
			BufSum += isqrt(R + I);

			//Calculate the index for the column
			Index = (Cnt*XPix)>>(L2Len-1);

			//Reset BufSum once the index has changed
			if(Index != IndO){
				//Write buffer sum to the column. Scale BufSum dependent on
				//input amplitude range. If 'ColumnFilter' is more than 0,
				//low pass filter the value to smooth out erratic changes.
				if(Decibels){
					Col[Index] += (int32_t)(20.0f*log10f(BufSum)-Col[Index])>>ColumnFilter;
				}
				else{
					Col[Index] += (BufSum-Col[Index])>>ColumnFilter;
				}

				IndO = Index;
				BufSum = 0;
			}
		}

		//Clear the current buffer
		ClrBuf();

		//Write each column value to the columns
		for(Cnt = 1 ; Cnt<XPix; Cnt++){
			DrawCol(Cnt, Col[Cnt]);
		}

		//Print screen!
		PScrn();

		//Reset the data obtained flag
		DataGot = 0;
	}
}
