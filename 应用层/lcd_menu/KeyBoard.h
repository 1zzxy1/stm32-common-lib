/*
 * KeyBoard.h
 *
 *  Created on: Apr 28, 2016
 *      Author: MJY
 */
#ifndef  _KeyBbord_H
#define  _KeyBbord_H

#include "sys.h"

extern uint8_t key;

//LCD X坐标（8*16），一共240/8=30个字一行
#define bit_col     30
#define bit1        0
#define bit2        bit1+bit_col
#define bit3        bit2+bit_col
#define bit4        bit3+bit_col
#define bit5        bit4+bit_col
#define bit6        bit5+bit_col
#define bit7        bit6+bit_col
#define X(x)        x*bit_col - bit_col
#define LCDXEND    X(7)

#define Key_Up           PDin(3)
#define Key_Down         PDin(0)
#define Key_Rignt        PDin(2)
#define Key_Center       PDin(1)
#define Key_Left         PDin(4)

typedef enum{
	pint8_t,
	puint8_t,
	pint16_t,
	puint16_t,
  pint32_t,
  puint32_t,
	pfloat,
	pdouble,
}parameter_type;
 
void Key_IO_Init(void);
//uint8_t GetKeyInpt(void);
uint8_t KEY_Scan(void);

void LCD_ConfigParameter(void);
void clean_invalid_char(uint8_t endx);
void ChangeParameterVal(parameter_type ptype,void* ppara,double precision,uint8_t keyx);

void ChangeFlagVal(uint8_t *flag,uint8_t keyx,uint8_t min,uint8_t max);

#endif

