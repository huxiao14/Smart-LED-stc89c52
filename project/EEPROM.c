#include <reg51.h>
#include <intrins.h>
#include <EEPROM.h>

/*
512B*10
1:2000-21FF
2:2200-23FF
3:2400-25FF  储存Mode
4:2600-27FF  储存NM的低中高亮度,2600为高位 2601为低位
5:2800-29FF  储存SM的Lux设置
6:2A00-2BFF
7:2C00-2DFF   
8:2E00-2FFF   
9:3000-31FF			
10:3200-33FF

*/

//默认未写入时为0xFF

void IAP_Idle()
{
	IAP_CONTR = 0;
	IAP_CMD = 0;
	IAP_TRIG = 0;
	IAP_ADDRH = 0x80;
	IAP_ADDRL = 0;
}

void IAP_TrigUnit()
{
	IAP_TRIG = 0x46;
	IAP_TRIG = 0xB9;
}

unsigned char IAP_ReadByte(unsigned int addr)
{
	unsigned char dat;
	IAP_CONTR = ENABLE_IAP;
	IAP_CMD = CMD_READ;
	IAP_ADDRL = addr;
	IAP_ADDRH = addr >> 8;
	IAP_TrigUnit();
	_nop_();
	dat = IAP_DATA;
	IAP_Idle();
	
	return dat;
}

void IAP_ProgramByte(unsigned int addr,unsigned char dat) //考虑是否需要关中断
{
//	EA = 0; //这里因为中断比较多最好还是关了
	IAP_CONTR = ENABLE_IAP;
	IAP_CMD = CMD_PROGRAM;
	IAP_ADDRL = addr;
	IAP_ADDRH = addr >> 8;
	IAP_DATA = dat;
	IAP_TrigUnit();
	_nop_();
	IAP_Idle();
	EA = 1;
}

void IAP_EraseSector(unsigned int addr) //注意写入前理应要擦除，且擦除按扇区，若有部分需要保存则需要先拷入ram中
{
//	EA = 0;
	IAP_CONTR = ENABLE_IAP;
	IAP_CMD = CMD_ERASE;
	IAP_ADDRL = addr;
	IAP_ADDRH = addr >> 8;
	IAP_TrigUnit();
	_nop_();
	IAP_Idle();
	EA = 1;
}

