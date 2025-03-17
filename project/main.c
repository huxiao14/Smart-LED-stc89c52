#include <reg51.h>
#include <intrins.h>
#include <BH.h>
#include <Timer.h>
#include <EEPROM.h>
#include <oled.h>
#include <St.h>

#define KP 5		//比例系数
#define KI 1		//积分系数
#define KD 3		//微分系数
#define DT 2 	//  (1/0.5)  单次采样时间为0.1s,使用倒数
#define pow_KP 4 //右移的幂次，也就是负幂次，这边以2为底的幂次作为定点数幂次，其实也可以选用10的倍数做*/应该效率也不会差太多。
#define pow_KI 10
#define pow_KD 8
#define COUNT0_SET 200 		//2us*1000=2ms,500hz
#define LOW_LIGHTSET 120 	//lightset越大，LED亮度越低（pwm设置的原因）
#define MID_LIGHTSET 60
#define ALLOWABLE_ERROR 2 //智能模式时允许的误差


/*enabled delay function:
delay_nms()
Delay_5us()
Delay_5ms()
*/
/*
所有OLED只显示字号大小为16的字体，字库中已将小字号删除
*/

/*
引脚定义：
				P0.0-P0.1:OLED
				P1.0-P1.1:BH1750
				P1.2:LED
				P2.0-2.2:MODE UP DOWN
*/

const unsigned char *Space="    ",*NM = "Normal Mode",*SM = "Smart Mode",*NM_High="High",*NM_Mid="Mid",*NM_Low="Low",*LIGHT="Lux=",*BN = "Brightness:",*WL="Lux_Set=",*SL="Smart Lamp:)",*WE="Welcome",*HX="made by Hu Xiao";

bit UP_sign=0,DOWN_sign=0,MODE_sign=0,m=0;
unsigned char i=0,mode;
unsigned char wish_lux[]={'0','0','3','0','0','\0'};
unsigned int count0=0,light_set=0,count1=0,error=0,error_sum=0,error_last=0,filtered_light_set = 0;
BH_data *lux;
unsigned char *wish_lux_p = &wish_lux;

sbit LED = P1^2;
sbit MODE = P2^0;
sbit UP = P2^1;
sbit DOWN = P2^2;

void Normal_Mode();
void Smart_Mode();
//unsigned int pow(unsigned char m,unsigned char n);

void main()
{
	delay_nms(10);
	BH1750_Init();
	lux = BH1750_Read();			//防止lux成为野指针
	OLED_Init();
	OLED_Clear();
	OLED_ShowString(35,2,WE,16);  //显示欢迎语
	OLED_ShowString(0,4,HX,16);
	delay_nms(2000); //delay
	OLED_Clear();
	delay_nms(10);
	for (;;){               //主循环
		mode = IAP_ReadByte(0x2400);     //读取上次关机时的mode
		if (mode!=0xFF){         //不是第一次开机时
			if (mode==0)Normal_Mode();//进入对应的模式，每次mode切换时，实际为更改0x2400的值，然后再进入主循环中读取并进行判断
			else if (mode==1)Smart_Mode();
		}
		else Normal_Mode();   //第一次开机默认进入普通模式
	}
}

void Timer0_Do() interrupt 1 using 1    //计时器0的执行函数，在LED口上生成PWM波形 500hz
{
	count0++;
	if (count0>=COUNT0_SET){
		count0=0;
	}                  
	if (count0<light_set)LED = 0;
	else if (count0>=light_set)LED = 1;
	TH0 = 0xFF;
	TL0 = 0xF7;
}

void Timer1_Do() interrupt 3 using 2 //计时器1只是单纯改变count1，测量在mode函数中进行，为防止测量函数在中断内外同时执行，会导致崩溃
{
	count1++;
	if (count1>=10){
		count1 = 0;
//		BH1750_Init();
//		lux = BH1750_Read(); //每次测试后都要重新上电
	}
	TH1 = 0x3C;
	TL1 = 0xB0;
}

void Timer2_Do() interrupt 5 using 3 			//计时器2负责按键检测，计时器2自动重载
{
	TF2 = 0;   //置0
	if (UP==0)UP_sign = 1;
	if (DOWN==0)DOWN_sign = 1;
	if (MODE==0)MODE_sign = 1;
}

void Normal_Mode()
{
	unsigned char brightness=0; //0 high 1 mid 2 low
	unsigned char temp1,temp2;
	OLED_ShowString(0,0,SL,16);
	OLED_ShowString(0,2,NM,16);   //不做改变的显示内容只显示一次
	OLED_ShowString(0,4,BN,16);
	OLED_ShowString(0,6,LIGHT,16);
	temp1 = IAP_ReadByte(0x2600);
	temp2 = IAP_ReadByte(0x2601); //读取保存的lightset值
	if (temp1!=0xFF){
		light_set |= (unsigned int)temp1;
		light_set |= (((unsigned int)temp2)<<8); //分字节读取后合成lightset
		if (light_set==0){
			OLED_ShowString(88,4,NM_High,16);
		}
		else if (light_set==MID_LIGHTSET){
			brightness = 1;
			OLED_ShowString(88,4,NM_Mid,16);
		}
		else if (light_set==LOW_LIGHTSET){
			brightness = 2;
			OLED_ShowString(88,4,NM_Low,16); //回到上次设定的光亮强度
		}
	}
	Timer0_Init();	
	Timer1_Init();     //初始化两个定时器
	Timer2_Init();
	for(;;){
		if (count1==1){
			BH1750_Init();
			lux = BH1750_Read();
			ET1 = 0;
			OLED_ShowString(32,6,lux->data_p,16);
			ET1 = 1;																//普通模式下仅做照度表
		}
		if ((DOWN == 0||DOWN_sign==1)&&brightness!=2){
			if (brightness==1){					//brightness变量显示了亮度模式，0 1 2分别为high mid low
				brightness++;
				OLED_ShowString(88,4,Space,16);        //先显示空白进行覆盖，因为high mid low字符数量不同
				OLED_ShowString(88,4,NM_Low,16);
				count0 = 0;       //重置计数器
				light_set = LOW_LIGHTSET;
			}
			else if (brightness==0){
				brightness++;
				OLED_ShowString(88,4,Space,16);
				OLED_ShowString(88,4,NM_Mid,16);
				count0 = 0;
				light_set = MID_LIGHTSET;
			}
			IAP_EraseSector(0x2600);             //当发生亮度变化时才改写eeprom
			IAP_ProgramByte(0x2600,light_set);
			IAP_ProgramByte(0x2601,light_set>>8);       //0x2601保存lightset的高位
			DOWN_sign = 0;
		}
		while(DOWN==0);               //等待松开
		if ((UP == 0||UP_sign==1)&&brightness!=0){
			if (brightness==1){
				brightness--;
				OLED_ShowString(88,4,Space,16);
				OLED_ShowString(88,4,NM_High,16);
				count0 = 0;
				light_set = 0;
			}
			else if (brightness==2){
				brightness--;
				OLED_ShowString(88,4,Space,16);
				OLED_ShowString(88,4,NM_Mid,16);
				count0 = 0;
				light_set = MID_LIGHTSET;
			}
			IAP_EraseSector(0x2600);
			IAP_ProgramByte(0x2600,light_set);
			IAP_ProgramByte(0x2601,light_set>>8);
			UP_sign = 0;
		}
		while(UP==0);
		if (MODE_sign)break;
	}
	Timer1_Stop();         //停止定时器
	Timer0_Stop();
	Timer2_Stop();
	light_set=0;
	count0=0;
	count1=0;
	UP_sign=0;
	DOWN_sign=0;
	MODE_sign=0;
	IAP_EraseSector(0x2400);
	IAP_ProgramByte(0x2400,1);    //1 means Smart Mode
	OLED_Clear();
	delay_nms(100);
}

void lux_plus(unsigned char op)          //对设定亮度值的更改，直接对字符数组操作
{
	unsigned char plus_i;
	if (op=='+'){
		wish_lux[4]++;
		for (plus_i=4;plus_i>=1;plus_i--){
			if (wish_lux[plus_i]==58){  //'9'+1 == 58
				wish_lux[plus_i] = '0';
				wish_lux[plus_i-1]++;
			}
		}
	}
	else if (op=='-'){
		wish_lux[4]--;
		for (plus_i=4;plus_i>=1;plus_i--){
			if (wish_lux[plus_i]==47){  //'0'-1 == 47
				wish_lux[plus_i] = '9';
				wish_lux[plus_i-1]--;
			}
		}
	}
}

unsigned int abs_minus(unsigned int a,unsigned int b) //返回两数的大减小
{
	if (a>b)return(a-b);
	else if (a<b)return(b-a);
	return 0;
}

void PI_Control(unsigned char *wl)       //核心的PID控制算法函数
{
	unsigned int PI_wl=0,delta,derivative,pow_wl=10000;
	unsigned char PI_i;
	for (PI_i=0;PI_i<=4;PI_i++){
		PI_wl += (wl[PI_i]-'0')*pow_wl;
		pow_wl/=10;                             //先将字符数组中的值读取为unsigned int
	}
	if (PI_wl==(lux->data_num)){             //恰等于此时下方照度即可结束函数
		return;
	}
	else{
		error = abs_minus(PI_wl,(lux->data_num)); //abs_minus()可返回两数相减的绝对值							                 //由于PID算法在这里控制的是lightset的变化量大小，所以全部取绝对值
		if (error<=ALLOWABLE_ERROR)return;
		error_sum += (error/DT);
		derivative = abs_minus(error,error_last)*DT; //DT在此已经过倒数处理
		delta = ((KP*error)>>pow_KP) + ((KI*error_sum)>>pow_KI)+((KD*derivative)>>pow_KD);  //定点数处理PID输出量  //记得加括号
		if (PI_wl>(lux->data_num)){
			if(light_set>=delta){light_set-=delta;}
			else light_set = 0;           //无法达到设定照度时将LED设为最大亮度
			delay_nms(60);
		}
		else if (PI_wl<(lux->data_num)){
			if(light_set+delta<=COUNT0_SET){light_set += delta;} 
			else light_set = COUNT0_SET; //外界光过强时LED熄灭
		}
	}
	error_last = error;          //更新error_last
}

void Smart_Mode()
{
	unsigned char temp,j;
	unsigned int k=0;
	OLED_ShowString(0,0,SL,16);
	OLED_ShowString(0,2,SM,16);
	OLED_ShowString(0,4,WL,16);
	OLED_ShowString(0,6,LIGHT,16);
	for (i=0;i<=5;i++){
		temp = IAP_ReadByte(0x2800+i);
		if (temp!=0xFF)wish_lux[i] = temp;
	}
	OLED_ShowString(64,4,wish_lux_p,16);
	Timer0_Init();
	Timer1_Init();
	Timer2_Init();
	for(;;){
//		OLED_ShowString(64,4,wish_lux_p,16);       //每个循环重新显示一次期望照度
		if (count1==9){
			BH1750_Init();
			lux = BH1750_Read();
			ET1 = 0;
			OLED_ShowString(32,6,lux->data_p,16);
			PI_Control(wish_lux_p);        //在这里进行控制
			ET1 = 1;
		}
		while(UP==0||UP_sign==1){
			if (UP==0&&wish_lux[0]<'5'){        //这里wish_lux<50000是做了简单处理，只有在阳光下才能达到这个照度，而且此时改变灯的亮度对改变照度没有任何意义
				if(k<=5){           //k其实为希望照度改变量，通过k的值来判断长按后的动作
					lux_plus('+');
					k++;
				}
				else if (k>5&&k<=20){
					j=2;
					while(j--){lux_plus('+');k++;}
				}
				else if (k>20&&k<=50){
					j=5;
					while(j--){lux_plus('+');k++;}
				}
				else if (k>50){
					j=20;
					while(j--){lux_plus('+');k++;}
				}
			}
			UP_sign = 0;
			error_sum = 0;   //每次改变完将累计误差和上次误差都置0
			error_last = 0;
			OLED_ShowString(64,4,wish_lux_p,16); //更新希望照度显示
		}
		k=0;
		while(DOWN==0||DOWN_sign==1){
			if (DOWN==0&&(((wish_lux[0]-'0')|(wish_lux[1]-'0')|(wish_lux[2]-'0')|(wish_lux[3]-'0')|(wish_lux[4]-'0'))!=0)){				//各位不全为0
				if(k<=5){
					lux_plus('-');
					k++;
				}
				else if (k>5&&k<=20){
					j=2;
					while(j--){lux_plus('-');k++;}
				}
				else if (k>20&&k<=50){
					j=5;
					while(j--){lux_plus('-');k++;}
				}
				else if (k>50){
					j=20;
					while(j--){lux_plus('-');k++;}
				}
			}
			DOWN_sign = 0;
//			if (k>=5){
//				OLED_ShowString(64,4,wish_lux_p,16);
//				k=0;
//			}
//			k++;
//			DOWN_sign=0;
			error_sum = 0;
			error_last = 0;
			OLED_ShowString(64,4,wish_lux_p,16);
		}
//		if (k!=0){             //若有改变，重新写入希望照度
//			IAP_EraseSector(0x2800);
//			for (i=0;i<=5;i++)IAP_ProgramByte(0x2800+i,wish_lux[i]);
//			k = 0;
//			UP_sign=0;
//			DOWN_sign=0;
//		}
		k=0;
		if (MODE_sign)break;
	}
	Timer1_Stop();
	Timer0_Stop();
	Timer2_Stop();
	light_set=0;
	IAP_EraseSector(0x2800);
	for (i=0;i<=5;i++)IAP_ProgramByte(0x2800+i,wish_lux[i]);
	count0=0;
	count1=0;
	UP_sign=0;
	DOWN_sign=0;
	MODE_sign=0;
	IAP_EraseSector(0x2400);
	IAP_ProgramByte(0x2400,0);    //0 means Normal Mode
	OLED_Clear();
	delay_nms(100);
}

//unsigned int pow(unsigned char m,unsigned char n)
//{
//	unsigned char l=n;
//	unsigned int result=1;	 
//	while(l--)result*=m;    
//	return result;
//}			