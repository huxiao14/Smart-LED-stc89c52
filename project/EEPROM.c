#include <reg51.h>
#include <intrins.h>
#include <EEPROM.h>

/*
512B*10
1:2000-21FF
2:2200-23FF
3:2400-25FF  ����Mode
4:2600-27FF  ����NM�ĵ��и�����,2600Ϊ��λ 2601Ϊ��λ
5:2800-29FF  ����SM��Lux����
6:2A00-2BFF
7:2C00-2DFF   
8:2E00-2FFF   
9:3000-31FF			
10:3200-33FF

*/

//Ĭ��δд��ʱΪ0xFF

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

void IAP_ProgramByte(unsigned int addr,unsigned char dat) //�����Ƿ���Ҫ���ж�
{
//	EA = 0; //������Ϊ�жϱȽ϶���û��ǹ���
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

void IAP_EraseSector(unsigned int addr) //ע��д��ǰ��ӦҪ�������Ҳ��������������в�����Ҫ��������Ҫ�ȿ���ram��
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

