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

//void Timer_Tran(unsigned int time_us,unsigned char *p_high,unsigned char *p_low) //将定时器所需时间以us形式传入，并将指针指向的high low改为可直接设置定时器的值
//{
//	unsigned int time = 65535-time_us+1;
//	unsigned char tmp;
//	tmp = (unsigned char)(time & 0x00FF);
//	*p_low = tmp;
//	tmp = (unsigned char)(time >> 8);
//	*p_high = tmp;
//}

void Timer0_Init()		//初始化定时器0   10us
{
	TMOD &= 0xF0; // 保持定时器1不变的前提下清空定时器0的设置
	TMOD |= 0x01; // 设置定时器模式为xxxx0001，即定时器0的模式1
	TH0 = 0xFF;		// 定时器高位设置	
	TL0 = 0xF7;		// 定时器低位设置
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

void Timer1_Init()		//初始化定时器1 50ms
{
	TMOD &= 0x0F; // 保持定时器0不变的前提下清空定时器1的设置
	TMOD |= 0x10; // 设置定时器模式为0001xxxx，即定时器1的模式1
	TH1 = 0x3C;		// 定时器高位设置	
	TL1 = 0xB0;		// 定时器低位设置
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
//void Timer1_Init(unsigned char *p_high,unsigned char *p_low)		//初始化定时器1
//{
//	TMOD &= 0x0F; // 保持定时器0不变的前提下清空定时器1的设置
//	TMOD |= 0x10; // 设置定时器模式为0001xxxx，即定时器1的模式1
//	TH1 = *p_high;		// 定时器高位设置	
//	TL1 = *p_low;		// 定时器低位设置
//	TR1 = 1;		
//	ET1 = 1;
//	EA = 1;
//}

void Timer2_Init()   //25ms,16位自动重载
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