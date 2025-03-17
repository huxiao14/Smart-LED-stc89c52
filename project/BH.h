#include <reg51.h>
#include <intrins.h>
#include <St.h>
#ifndef __BH_H
#define __BH_H

#define   uchar unsigned char
#define   uint unsigned int	
	
//typedef   unsigned char BYTE;
//typedef   unsigned short WORD;


void delay_nms(unsigned int k);
void Delay5us();
void Delay5ms();
void BH1750_Start();
void BH1750_Stop();
void BH1750_SendACK(bit ack);
bit BH1750_RecvACK();
void BH1750_SendByte(uchar dat);
uchar BH1750_RecvByte();
void BH1750_Write(uchar REG_Address);
BH_data *BH1750_Read(void);
void BH1750_Init();

#endif