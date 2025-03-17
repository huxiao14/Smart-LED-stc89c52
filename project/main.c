#include <reg51.h>
#include <intrins.h>
#include <BH.h>
#include <Timer.h>
#include <EEPROM.h>
#include <oled.h>
#include <St.h>

#define KP 5		//����ϵ��
#define KI 1		//����ϵ��
#define KD 3		//΢��ϵ��
#define DT 2 	//  (1/0.5)  ���β���ʱ��Ϊ0.1s,ʹ�õ���
#define pow_KP 4 //���Ƶ��ݴΣ�Ҳ���Ǹ��ݴΣ������2Ϊ�׵��ݴ���Ϊ�������ݴΣ���ʵҲ����ѡ��10�ı�����*/Ӧ��Ч��Ҳ�����̫�ࡣ
#define pow_KI 10
#define pow_KD 8
#define COUNT0_SET 200 		//2us*1000=2ms,500hz
#define LOW_LIGHTSET 120 	//lightsetԽ��LED����Խ�ͣ�pwm���õ�ԭ��
#define MID_LIGHTSET 60
#define ALLOWABLE_ERROR 2 //����ģʽʱ��������


/*enabled delay function:
delay_nms()
Delay_5us()
Delay_5ms()
*/
/*
����OLEDֻ��ʾ�ֺŴ�СΪ16�����壬�ֿ����ѽ�С�ֺ�ɾ��
*/

/*
���Ŷ��壺
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
	lux = BH1750_Read();			//��ֹlux��ΪҰָ��
	OLED_Init();
	OLED_Clear();
	OLED_ShowString(35,2,WE,16);  //��ʾ��ӭ��
	OLED_ShowString(0,4,HX,16);
	delay_nms(2000); //delay
	OLED_Clear();
	delay_nms(10);
	for (;;){               //��ѭ��
		mode = IAP_ReadByte(0x2400);     //��ȡ�ϴιػ�ʱ��mode
		if (mode!=0xFF){         //���ǵ�һ�ο���ʱ
			if (mode==0)Normal_Mode();//�����Ӧ��ģʽ��ÿ��mode�л�ʱ��ʵ��Ϊ����0x2400��ֵ��Ȼ���ٽ�����ѭ���ж�ȡ�������ж�
			else if (mode==1)Smart_Mode();
		}
		else Normal_Mode();   //��һ�ο���Ĭ�Ͻ�����ͨģʽ
	}
}

void Timer0_Do() interrupt 1 using 1    //��ʱ��0��ִ�к�������LED��������PWM���� 500hz
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

void Timer1_Do() interrupt 3 using 2 //��ʱ��1ֻ�ǵ����ı�count1��������mode�����н��У�Ϊ��ֹ�����������ж�����ͬʱִ�У��ᵼ�±���
{
	count1++;
	if (count1>=10){
		count1 = 0;
//		BH1750_Init();
//		lux = BH1750_Read(); //ÿ�β��Ժ�Ҫ�����ϵ�
	}
	TH1 = 0x3C;
	TL1 = 0xB0;
}

void Timer2_Do() interrupt 5 using 3 			//��ʱ��2���𰴼���⣬��ʱ��2�Զ�����
{
	TF2 = 0;   //��0
	if (UP==0)UP_sign = 1;
	if (DOWN==0)DOWN_sign = 1;
	if (MODE==0)MODE_sign = 1;
}

void Normal_Mode()
{
	unsigned char brightness=0; //0 high 1 mid 2 low
	unsigned char temp1,temp2;
	OLED_ShowString(0,0,SL,16);
	OLED_ShowString(0,2,NM,16);   //�����ı����ʾ����ֻ��ʾһ��
	OLED_ShowString(0,4,BN,16);
	OLED_ShowString(0,6,LIGHT,16);
	temp1 = IAP_ReadByte(0x2600);
	temp2 = IAP_ReadByte(0x2601); //��ȡ�����lightsetֵ
	if (temp1!=0xFF){
		light_set |= (unsigned int)temp1;
		light_set |= (((unsigned int)temp2)<<8); //���ֽڶ�ȡ��ϳ�lightset
		if (light_set==0){
			OLED_ShowString(88,4,NM_High,16);
		}
		else if (light_set==MID_LIGHTSET){
			brightness = 1;
			OLED_ShowString(88,4,NM_Mid,16);
		}
		else if (light_set==LOW_LIGHTSET){
			brightness = 2;
			OLED_ShowString(88,4,NM_Low,16); //�ص��ϴ��趨�Ĺ���ǿ��
		}
	}
	Timer0_Init();	
	Timer1_Init();     //��ʼ��������ʱ��
	Timer2_Init();
	for(;;){
		if (count1==1){
			BH1750_Init();
			lux = BH1750_Read();
			ET1 = 0;
			OLED_ShowString(32,6,lux->data_p,16);
			ET1 = 1;																//��ͨģʽ�½����նȱ�
		}
		if ((DOWN == 0||DOWN_sign==1)&&brightness!=2){
			if (brightness==1){					//brightness������ʾ������ģʽ��0 1 2�ֱ�Ϊhigh mid low
				brightness++;
				OLED_ShowString(88,4,Space,16);        //����ʾ�հ׽��и��ǣ���Ϊhigh mid low�ַ�������ͬ
				OLED_ShowString(88,4,NM_Low,16);
				count0 = 0;       //���ü�����
				light_set = LOW_LIGHTSET;
			}
			else if (brightness==0){
				brightness++;
				OLED_ShowString(88,4,Space,16);
				OLED_ShowString(88,4,NM_Mid,16);
				count0 = 0;
				light_set = MID_LIGHTSET;
			}
			IAP_EraseSector(0x2600);             //���������ȱ仯ʱ�Ÿ�дeeprom
			IAP_ProgramByte(0x2600,light_set);
			IAP_ProgramByte(0x2601,light_set>>8);       //0x2601����lightset�ĸ�λ
			DOWN_sign = 0;
		}
		while(DOWN==0);               //�ȴ��ɿ�
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
	Timer1_Stop();         //ֹͣ��ʱ��
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

void lux_plus(unsigned char op)          //���趨����ֵ�ĸ��ģ�ֱ�Ӷ��ַ��������
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

unsigned int abs_minus(unsigned int a,unsigned int b) //���������Ĵ��С
{
	if (a>b)return(a-b);
	else if (a<b)return(b-a);
	return 0;
}

void PI_Control(unsigned char *wl)       //���ĵ�PID�����㷨����
{
	unsigned int PI_wl=0,delta,derivative,pow_wl=10000;
	unsigned char PI_i;
	for (PI_i=0;PI_i<=4;PI_i++){
		PI_wl += (wl[PI_i]-'0')*pow_wl;
		pow_wl/=10;                             //�Ƚ��ַ������е�ֵ��ȡΪunsigned int
	}
	if (PI_wl==(lux->data_num)){             //ǡ���ڴ�ʱ�·��նȼ��ɽ�������
		return;
	}
	else{
		error = abs_minus(PI_wl,(lux->data_num)); //abs_minus()�ɷ�����������ľ���ֵ							                 //����PID�㷨��������Ƶ���lightset�ı仯����С������ȫ��ȡ����ֵ
		if (error<=ALLOWABLE_ERROR)return;
		error_sum += (error/DT);
		derivative = abs_minus(error,error_last)*DT; //DT�ڴ��Ѿ�����������
		delta = ((KP*error)>>pow_KP) + ((KI*error_sum)>>pow_KI)+((KD*derivative)>>pow_KD);  //����������PID�����  //�ǵü�����
		if (PI_wl>(lux->data_num)){
			if(light_set>=delta){light_set-=delta;}
			else light_set = 0;           //�޷��ﵽ�趨�ն�ʱ��LED��Ϊ�������
			delay_nms(60);
		}
		else if (PI_wl<(lux->data_num)){
			if(light_set+delta<=COUNT0_SET){light_set += delta;} 
			else light_set = COUNT0_SET; //�����ǿʱLEDϨ��
		}
	}
	error_last = error;          //����error_last
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
//		OLED_ShowString(64,4,wish_lux_p,16);       //ÿ��ѭ��������ʾһ�������ն�
		if (count1==9){
			BH1750_Init();
			lux = BH1750_Read();
			ET1 = 0;
			OLED_ShowString(32,6,lux->data_p,16);
			PI_Control(wish_lux_p);        //��������п���
			ET1 = 1;
		}
		while(UP==0||UP_sign==1){
			if (UP==0&&wish_lux[0]<'5'){        //����wish_lux<50000�����˼򵥴���ֻ���������²��ܴﵽ����նȣ����Ҵ�ʱ�ı�Ƶ����ȶԸı��ն�û���κ�����
				if(k<=5){           //k��ʵΪϣ���նȸı�����ͨ��k��ֵ���жϳ�����Ķ���
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
			error_sum = 0;   //ÿ�θı��꽫�ۼ������ϴ�����0
			error_last = 0;
			OLED_ShowString(64,4,wish_lux_p,16); //����ϣ���ն���ʾ
		}
		k=0;
		while(DOWN==0||DOWN_sign==1){
			if (DOWN==0&&(((wish_lux[0]-'0')|(wish_lux[1]-'0')|(wish_lux[2]-'0')|(wish_lux[3]-'0')|(wish_lux[4]-'0'))!=0)){				//��λ��ȫΪ0
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
//		if (k!=0){             //���иı䣬����д��ϣ���ն�
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