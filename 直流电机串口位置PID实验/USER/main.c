#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "usart.h"
#include "timer.h"
#include "lcd.h"
#include "motor.h"
#include "v_pid.h"
#include "p_pid.h"
#include "stm32f10x_fsmc.h"
/************************************************
 ALIENTEKս��STM32������ʵ��8
 ��ʱ���ж�ʵ��
 ����֧�֣�www.openedv.com
 �Ա����̣�http://eboard.taobao.com 
 ��ע΢�Ź���ƽ̨΢�źţ�"����ԭ��"����ѻ�ȡSTM32���ϡ�
 ������������ӿƼ����޹�˾  
 ���ߣ�����ԭ�� @ALIENTEK
************************************************/

/***************************************************

        �ٶ�PID���޸������ܳ�PID�����һ��bug
				
				Ȼ�󣬶���һ��bug 
				
				���������ģ��PID������ר��PID

         Ӳ�����ӣ� ������  A6-PWM4 A7-PWM3 GND-GND
			               ���	 ��-OUT1  ��-OUT2
										 ������ :B6-B   B7-A
                      ����1��PC
																				��
																				2018/10/5

******************************************************/
#define VMAX  33 
int Count=10000,Old_Could=10000;
long Real_Position=0;
int now_velocity=0;
int set_velocity=0;
int pwm1=0,pwm2=0;  //��������ʾ
extern short int PID_m_add;
extern int PID_P_add; 
int new_flag=0;

volatile long Set_Position=0;
extern int Max_V;
 int main(void)
 {		
 	u16 t,i;
	 long num,sendnum;  
	u16 len;	
	
	delay_init();	    	 //��ʱ������ʼ��	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	uart_init(9600);	 //���ڳ�ʼ��Ϊ115200
 	LED_Init();			     //LED�˿ڳ�ʼ��
 
  LCD_Init();	
	POINT_COLOR=BLUE;																		//������ɫ
	TIM3_PWM_Init(3559,0);
	TIM4_Int_Init();  
	TIM2_Int_Init(9,7199);  
	 pwm1=0;
	 pwm2=0;
   	while(1)
	{
	
		if(now_velocity>=0)															
			LCD_ShowString(30,150,200,16,16,"velocity:   ");
		 else
			LCD_ShowString(30,150,200,16,16,"velocity: -");	
		 LCD_ShowNum(150,150,abs(now_velocity),4,16);
		 if(Real_Position>=0)													
					LCD_ShowString(30,200,200,16,16,"Position:   ");
		 else
			LCD_ShowString(30,200,200,16,16,"Position:-");	
		 LCD_ShowNum(150,200,abs(Real_Position),6,16);
		 
		 
		 if(set_velocity>=0)	
		 LCD_ShowString(30,80,200,16,16,"set_velocity:  ");
		  else
		  LCD_ShowString(30,80,200,16,16,"set_velocity: -");
				pwm2=abs(set_velocity);
		 LCD_ShowNum(150,80,pwm2,4,16);
		 
		 
		 if(Set_Position>=0)	
		 LCD_ShowString(30,100,200,16,16,"Set_Position:  ");
		  else
		 LCD_ShowString(30,100,200,16,16,"Set_Position: -");
			pwm1=abs(Set_Position);
		 LCD_ShowNum(150,100,pwm1,6,16);
			
		//		printf("�������趨λ��,�Իس�������\n");  
			if(USART_RX_STA&0x8000)
		{					   
			len=USART_RX_STA&0x3fff;//�õ��˴ν��յ������ݳ���
		//	printf("\r\n�����͵���ϢΪ:\r\n\r\n");
			for(t=0;t<len;t++)
			{
				USART_SendData(USART1, USART_RX_BUF[t]);//�򴮿�1��������
				while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//�ȴ����ͽ���
			}
			
		//	printf("\nlen is %d\n",len);
		
			if(USART_RX_BUF[0]=='-')
			{
				sendnum=0;
				for(t=1;t<len;t++)
				{	
				
					num=USART_RX_BUF[t]-48;
					for(i=0;i<len-1-t;i++)
					{
						num*=10;
					}
					sendnum+=num;
				}
				sendnum=-sendnum;
				Set_Position=sendnum;
				new_flag=1;
			}
			else
			{  
				sendnum=0;
				for(t=0;t<len;t++)
				{	
					
					num=USART_RX_BUF[t]-48;
					for(i=0;i<len-1-t;i++)
					{
						num*=10;
					}
					sendnum+=num;
				}
					Set_Position=sendnum;
				new_flag=1;
			}
		//	printf("\r\n\r\n");//���뻻��
				
			printf("\nSet_Position is %ld�\\n",Set_Position);
		//	printf("����������,�Իس�������\n"); 
			USART_RX_STA=0;
		}
	}	 

 
}	 
 



































