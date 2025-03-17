#include <reg51.h>

sbit ET2 = IE^5;
sfr T2CON = 0xc8;
sbit TF2 = T2CON^7;
sbit TR2 = T2CON^2;
sfr T2MOD = 0xc9;
sfr RCAP2L = 0xca;
sfr RCAP2H = 0xcb;
sfr TL2 = 0xcc;
sfr TH2 = 0xcd;

//void Timer_Tran(unsigned int time_us,unsigned char *p_high,unsigned char *p_low) //����ʱ������ʱ����us��ʽ���룬����ָ��ָ���high low��Ϊ��ֱ�����ö�ʱ����ֵ
//{
//	unsigned int time = 65535-time_us+1;
//	unsigned char tmp;
//	tmp = (unsigned char)(time & 0x00FF);
//	*p_low = tmp;
//	tmp = (unsigned char)(time >> 8);
//	*p_high = tmp;
//}

void Timer0_Init()		//��ʼ����ʱ��0   10us
{
	TMOD &= 0xF0; // ���ֶ�ʱ��1�����ǰ������ն�ʱ��0������
	TMOD |= 0x01; // ���ö�ʱ��ģʽΪxxxx0001������ʱ��0��ģʽ1
	TH0 = 0xFF;		// ��ʱ����λ����	
	TL0 = 0xF7;		// ��ʱ����λ����
	TR0 = 1;
	ET0 = 1;
	EA = 1;
}

void Timer0_Stop()
{
	TR0 = 0;
	ET0 = 0;
//	EA = 0;
}

void Timer1_Init()		//��ʼ����ʱ��1 50ms
{
	TMOD &= 0x0F; // ���ֶ�ʱ��0�����ǰ������ն�ʱ��1������
	TMOD |= 0x10; // ���ö�ʱ��ģʽΪ0001xxxx������ʱ��1��ģʽ1
	TH1 = 0x3C;		// ��ʱ����λ����	
	TL1 = 0xB0;		// ��ʱ����λ����
	TR1 = 1;		
//	TF1 = 0;
	ET1 = 1;
	EA = 1;
}

void Timer1_Stop()
{
	TR1 = 0;
	ET1 = 0;
}
//void Timer1_Init(unsigned char *p_high,unsigned char *p_low)		//��ʼ����ʱ��1
//{
//	TMOD &= 0x0F; // ���ֶ�ʱ��0�����ǰ������ն�ʱ��1������
//	TMOD |= 0x10; // ���ö�ʱ��ģʽΪ0001xxxx������ʱ��1��ģʽ1
//	TH1 = *p_high;		// ��ʱ����λ����	
//	TL1 = *p_low;		// ��ʱ����λ����
//	TR1 = 1;		
//	ET1 = 1;
//	EA = 1;
//}

void Timer2_Init()   //25ms,16λ�Զ�����
{
	RCAP2L = TL2 = 0x58;
	RCAP2H = TH2 = 0x9E;
	TR2 = 1;
	ET2 = 1;
	EA = 1;
}

void Timer2_Stop()
{
	TR2 = 0;
	ET2 = 0;
}