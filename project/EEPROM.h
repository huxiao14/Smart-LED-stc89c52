#include <reg51.h>
#include <intrins.h>
#ifndef __EEPROM_H
#define __EEPROM_H

sfr IAP_DATA = 0xE2; //Flash data register
sfr IAP_ADDRH = 0xE3; //Flash address high
sfr IAP_ADDRL = 0xE4; //Flash address low
sfr IAP_CMD = 0xE5; //Flash command register
sfr IAP_TRIG = 0xE6; //Flash command trigger
sfr IAP_CONTR = 0xE7; //Flash control register

#define CMD_IDLE 0 //Stand by
#define CMD_READ 1 //Byte Read
#define CMD_PROGRAM 2 //Byte Program
#define CMD_ERASE 3 //Sector Erase

#define ENABLE_IAP 0x81

void IAP_Idle();
void IAP_TrigUnit();
unsigned char IAP_ReadByte(unsigned int addr);
void IAP_ProgramByte(unsigned int addr,unsigned char dat);
void IAP_EraseSector(unsigned int addr);

#endif
