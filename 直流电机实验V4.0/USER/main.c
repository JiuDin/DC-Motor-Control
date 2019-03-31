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
#include "adc.h"
#include "can.h"
#include "usb_lib.h"
#include "hw_config.h"
#include "usb_pwr.h"	 
/************************************************
 ALIENTEKս��STM32������ʵ��8
 ��ʱ���ж�ʵ��
 ����֧�֣�www.openedv.com
 �Ա����̣�http://eboard.taobao.com 
 ��ע΢�Ź���ƽ̨΢�źţ�"����ԭ��"����ѻ�ȡSTM32���ϡ�
 ������������ӿƼ����޹�˾  
 ���ߣ�����ԭ�� @ALIENTEK
************************************************/




/*******************************
���������Ƶĵ���������
Ӳ�����ӣ�  

�������˳ʱ�뷽��ת��Ϊ��
 M1  C7      Encoder1 A1  A6     
 M2  C6               B1  A7
 M3  C9      Encoder2 A2  B6
 M4  C8               B2  B7
 M5  A11      Encoder3 A3  A0
 M6  A8              B3  A1

ADC1   C0
ADC2   C1
ADC3   C2

M1   ������������Z �Աߵĵ�Դ��
M2   ���ߺ�ɫһ�˵ĵ�Դ��


***************************/

/***************************
 ��ʵ���ѻ���ʵ�ֵ���ֱ�����λ���ٶ�˫�����ƣ�λ���ٶȵ��������Ŀ����в��ȶ�
 ����Ҫ����PID�������㷨	

                  ������
									����2019/2/27
****************************/
/********************************

 ��ʵ�����ʵ�������������λ���ٶȿ��� ,�������ĵ������ƣ�Ч������
 ����PWM���ռ����CAN���ţ����CANͨѶ�����ض���
 Ӧʹ������PID����
 �������ڿ����ã�
 �Ż�~~~~
 
 
                      ������
											����2019/3/2




*******************************/
/*****************************
  ����һЩСС��BUG��
	�����������ݲ�ȷ���Ƿ�����ȶ�������
	������������~����δ�Ľ�Ӳ����·��
	λ�û��������� ����BUG������������ ������Ծٵ��ݣ�10UF ������δ�������⡣
//http://m.eepw.com.cn/article/201710/368839.html
   �����ˣ�����������������������������������������
           ������
					 ����2019/3/8
*******************************/
#define VMAX  33 
#define MYLCD 1
#define MYUSART 0
#define MYUSB 0  
//CAN��USB��Ӳ���ϳ�ͻ����Ҫʹ��CAN����CAN_RX0_INT_ENABLE��1��ͬʱ�Ƴ����޸�USB���������

int new_flag=0;//�ڴ���ͨѶ��������λ

volatile long Real_Position1=0;
volatile int now_velocity1=0;
volatile int now_current1;
int Initcurrent1;//��ʼ����  ���
volatile int set_velocity1=0;//���ڲ�������
volatile int Set_Current1=0;//���ڲ�������

volatile long Real_Position2=0;
volatile int now_velocity2=0;
volatile int now_current2;
int Initcurrent2;//��ʼ����  ���
volatile int set_velocity2=0;//���ڲ�������
volatile int Set_Current2=0;//���ڲ�������

volatile long Real_Position3=0;
volatile int now_velocity3=0;
volatile int now_current3;
int Initcurrent3;//��ʼ����  ���
volatile int set_velocity3=0;//���ڲ�������
volatile int Set_Current3=0;//���ڲ�������

extern int PID_v_add1;
extern int PID_i_add1;
extern int PID_i_add_set1;
extern int PID_P_add1; 
extern int PID_P_add_set1; 
extern int PID_v_add_set1; 

extern int PID_i_add2;
extern int PID_i_add_set2;
extern int PID_P_add2; 
extern int PID_P_add_set2; 
extern int PID_v_add_set2;

extern int PID_i_add3;
extern int PID_i_add_set3;
extern int PID_P_add3; 
extern int PID_P_add_set3; 
extern int PID_v_add_set3;


volatile int user_set_velocity1=8;//�ֶ��趨�ٶ�
volatile long user_Set_Position1=5000;//�ֶ��趨λ��
volatile int user_Set_Current1=100;//�趨������С������������
                              

volatile int user_set_velocity2=0;//�ֶ��趨�ٶ�
volatile long user_Set_Position2=0;//�ֶ��趨λ��
volatile int user_Set_Current2=100;


volatile int user_set_velocity3=0;//�ֶ��趨�ٶ�
volatile long user_Set_Position3=0;//�ֶ��趨λ��
volatile int user_Set_Current3=100;
															 
int MODE=5;  //0 ��λ�û�  1 ���ٶȻ� 2 ��������  
             //3 λ���ٶ�  4 �ٶȵ��� 5 λ���ٶȵ��� 
						 //6 ��������

int main(void)
 {	
	 u8 mo=0;//mo���ڼ��ģʽ�仯
	 u8 usbstatus=0;	//USB����״̬��־λ
   u16 t,i,len;
	 long num,sendnum;	
	  delay_init();	    	 //��ʱ������ʼ��	  
	  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	  uart_init(9600);	 //���ڳ�ʼ��Ϊ115200
	  LED_Init();			     //LED�˿ڳ�ʼ��
	 
	 if(MYUSB)
	 {
		  delay_ms(1800);
			USB_Port_Set(0); 	//USB�ȶϿ�
			delay_ms(700);
			USB_Port_Set(1);	//USB�ٴ�����
			Set_USBClock();   
			USB_Interrupts_Config();    
			USB_Init();	
	 }
 
	 
	if(MYLCD)
	{
		LCD_Init();
		POINT_COLOR=BLUE;																		//������ɫ
		LCD_ShowString(30,10,100,16,16,"MODE: ");
		LCD_ShowString(30,120,200,16,16,"ADC_CH0_VAL:");	      
//  LCD_ShowString(60,190,200,16,16,"Send Data:");		//��ʾ���͵�����	
//	LCD_ShowString(60,250,200,16,16,"Receive Data:");	//��ʾ���յ�������	
		LCD_ShowNum(150,10,MODE,4,16);
	
	}
	 
  
	//CAN_Mode_Init(CAN_SJW_1tq,CAN_BS2_8tq,CAN_BS1_9tq,4,0);
													

	 Adc_Init();	
	
	 Initcurrent1=3100; //ADC�ĳ�ʼ���븳��ֵ����TIM3��ʼ��֮ǰ
//	 Initcurrent2=Get_Adc_Average(ADC_Channel_2,100); 
//	 Initcurrent3=Get_Adc_Average(ADC_Channel_3,100); 
	 Initcurrent2=3100; 
	 Initcurrent3=3100; 
	 TIM1_PWM_Init(3559,0); //3599Ϊռ�ձ����ֵ
	 TIM8_PWM_Init(3559,0);
	
	 TIM2_Int_Init(9,7199);  
	 TIM3_Int_Init();  
	 TIM4_Int_Init(); 
   TIM5_Int_Init();  	
    Motor_pwm1_set(0);
	  Motor_pwm2_set(1000);
//	if(user_set_velocity1>=0)
//	  user_Set_Current1=Initcurrent1+100;  
//	else
//		 user_Set_Current1=Initcurrent1-100;  
//	if(user_set_velocity2>=0)
//	  user_Set_Current2=Initcurrent2+100;  
//	else
//		 user_Set_Current2=Initcurrent2-100;  
//	if(user_set_velocity3>=0)
//	  user_Set_Current3=Initcurrent3+100;  
//	else
//		 user_Set_Current3=Initcurrent3-100;  

	
	  mo=MODE;
				
  while(1)
	{
		if(MYLCD)
			{
				if(mo!=MODE)
						{
							mo=MODE;
							LCD_ShowNum(150,10,MODE,4,16);
						}
				LCD_ShowNum(150,10,MODE,4,16);
						
						
						if(Real_Position1>=0)	
				{					
						LCD_ShowString(30,60,200,16,16,"Position1:   ");
					  LCD_ShowNum(150,60,Real_Position1,6,16);
				}
					else
					{
						LCD_ShowString(30,60,200,16,16,"Position1: -");	
						LCD_ShowNum(150,60,-Real_Position1,6,16);
					}
					
				if(PID_i_add1>=0)	
				{					
						LCD_ShowString(30,80,200,16,16,"PID_i_add1:   ");
					LCD_ShowNum(150,80,PID_i_add1,6,16);
				}
					else
					{
						LCD_ShowString(30,80,200,16,16,"PID_i_add1: -");	
						LCD_ShowNum(150,80,-PID_i_add1,6,16);
					}
				
				if(user_Set_Current1>=0)	
				{					
						LCD_ShowString(30,100,200,16,16,"user_Set_Current1:   ");
					LCD_ShowNum(150,100,user_Set_Current1,6,16);
				}
					else
					{
						LCD_ShowString(30,100,200,16,16,"user_Set_Current1: -");	
						LCD_ShowNum(150,100,-user_Set_Current1,6,16);
					}

    
          if(Initcurrent1>=0)	
				{					
						LCD_ShowString(30,120,200,16,16,"Initcurrent1:   ");
					  LCD_ShowNum(150,120,Initcurrent1,6,16);
				}
					else
					{
						LCD_ShowString(30,120,200,16,16,"Initcurrent1: -");	
						LCD_ShowNum(150,120,-Initcurrent1,6,16);
					}
          if(now_velocity1>=0)	
				{					
						LCD_ShowString(30,140,200,16,16,"now_velocity1:   ");
					  LCD_ShowNum(150,140,now_velocity1,6,16);
				}
					else
					{
						LCD_ShowString(30,140,200,16,16,"now_velocity1: -");	
						LCD_ShowNum(150,140,-now_velocity1,6,16);
					}

					 if(now_current1>=0)	
				{					
						LCD_ShowString(30,160,200,16,16,"current:   ");
					  LCD_ShowNum(150,160,now_current1,6,16);
				}
					else
					{
						LCD_ShowString(30,160,200,16,16,"current: -");	
						LCD_ShowNum(150,160,-now_current1,6,16);
					}
          
					 if(now_velocity1>=0)	
				{					
						LCD_ShowString(30,180,200,16,16,"now_velocity1:   ");
					  LCD_ShowNum(150,180,now_velocity1,6,16);
				}
					else
					{
						LCD_ShowString(30,180,200,16,16,"now_velocity1: -");	
						LCD_ShowNum(150,180,-now_velocity1,6,16);
					}
          if(PID_v_add_set1>=0)	
				{					
						LCD_ShowString(30,200,200,16,16,"PID_v_add_set1:   ");
					  LCD_ShowNum(150,200,PID_v_add_set1,6,16);
				}
					else
					{
						LCD_ShowString(30,200,200,16,16,"PID_v_add_set1: -");	
						LCD_ShowNum(150,200,-PID_v_add_set1,6,16);
					}

					 if(PID_v_add1>=0)	
				{					
						LCD_ShowString(30,220,200,16,16,"PID_v_add1:   ");
					  LCD_ShowNum(150,220,PID_v_add1,6,16);
				}
					else
					{
						LCD_ShowString(30,220,200,16,16,"PID_v_add1: -");	
						LCD_ShowNum(150,220,-PID_v_add1,6,16);
					}
			    LCD_ShowxNum(156,240,now_current1,4,16,0);//��ʾADC��ֵ
					LCD_ShowxNum(156,260,now_current2,4,16,0);
					
					LCD_ShowxNum(156,280,now_current3,4,16,0);
			    LCD_ShowxNum(20,240,Set_Current1,4,16,0);//��ʾADC��ֵ
					LCD_ShowxNum(20,260,Set_Current2,4,16,0);
					LCD_ShowxNum(20,280,Set_Current3,4,16,0);
				}
	
	
//		usb_printf("\r\nnow_current IS %ld\r\n\r\n",now_current);
//		usb_printf("\rnow_current IS %ld\r\n\r\n",Set_Position);
//		usb_printf("\r4Real_Position IS %ld\r\n\r\n",Real_Position);
		
	
			
		now_current1=Get_Adc_Average(ADC_Channel_10,5);  //AD���������ʱ
  	
		now_current2=Get_Adc_Average(ADC_Channel_11,5);  
	
		now_current3=Get_Adc_Average(ADC_Channel_12,5);


//	  if(now_velocity==0)  //��ת����,,,,,���ּ�ⷽʽȷʵ������λ�û��������������˴����ĳ���
//		{                    //����������һ�ε�λ���ٵ��������򲻿�ȡ
//		  delay_ms(5);
//			if(now_velocity==0)
//			{
//				MODE=6;
//				Motor_pwm1_set(0);
//	    	Motor_pwm2_set(0);
//			}
//		}
			
  
		if(MYUSB)
			{
		if(usbstatus!=bDeviceState)//USB����״̬�����˸ı�.
		{
			usbstatus=bDeviceState;//��¼�µ�״̬
//			if(usbstatus==CONFIGURED)
//			{
//				POINT_COLOR=BLUE;
//				LCD_ShowString(30,130,200,16,16,"USB Connected    ");//��ʾUSB���ӳɹ�
//				LED1=0;//DS1��
//			}else
//			{
//				POINT_COLOR=RED;
//				LCD_ShowString(30,130,200,16,16,"USB disConnected ");//��ʾUSB�Ͽ�
//				LED1=1;//DS1��
//			}
		}
		if(USB_USART_RX_STA&0x8000)
		{					   
			len=USB_USART_RX_STA&0x3FFF;//�õ��˴ν��յ������ݳ���
			usb_printf("\r\n�����͵���ϢΪ:%d\r\n\r\n",len);
			for(t=0;t<len;t++)
			{
				USB_USART_SendData(USB_USART_RX_BUF[t]);//���ֽڷ�ʽ,���͸�USB 
			}
			usb_printf("\r\n\r\n");//���뻻��
			USB_USART_RX_STA=0;
		}	
	}
		
		
	

//    if(MYUSART)
//	 {	
//		 if(USART_RX_STA&0x8000)
//			{					   
//				len=USART_RX_STA&0x3fff;//�õ��˴ν��յ������ݳ���
//			//	printf("\r\n�����͵���ϢΪ:\r\n\r\n");
//				for(t=0;t<len;t++)
//					{
//							USART_SendData(USART1, USART_RX_BUF[t]);//�򴮿�1��������
//							while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//�ȴ����ͽ���
//					}
//			
//		  //	printf("\nlen is %d\n",len);
//		
//			if(USART_RX_BUF[0]=='a')
//			{
//				MODE=0;
//				sendnum=0;
//				for(t=1;t<len;t++)
//					{	
//				
//						num=USART_RX_BUF[t]-48;
//						for(i=0;i<len-1-t;i++)
//							{
//								num*=10;
//							}
//						sendnum+=num;
//					}
//			
//					user_Set_Position=sendnum;
//					new_flag=1;
//			}
//			if(USART_RX_BUF[0]=='b')
//			{
//				MODE=0;
//				sendnum=0;
//				for(t=1;t<len;t++)
//						{					
//							num=USART_RX_BUF[t]-48;
//							for(i=0;i<len-1-t;i++)
//								{
//										num*=10;
//								}
//							sendnum+=num;
//						}
//			  sendnum=-sendnum;
//				user_Set_Position=sendnum;
//				new_flag=1;
//			}
//			if(USART_RX_BUF[0]=='c')
//			{
//				MODE=1;
//				sendnum=0;
//				for(t=1;t<len;t++)
//					{				
//						num=USART_RX_BUF[t]-48;
//						for(i=0;i<len-1-t;i++)
//							{
//								num*=10;
//							}
//						sendnum+=num;
//					}			
//				user_set_velocity=sendnum;
//				new_flag=1;
//			}
//			if(USART_RX_BUF[0]=='d')
//				{
//						MODE=1;
//						sendnum=0;
//						for(t=1;t<len;t++)
//							{					
//								num=USART_RX_BUF[t]-48;
//								for(i=0;i<len-1-t;i++)
//										{
//											num*=10;
//										}
//						sendnum+=num;
//				}
//				sendnum=-sendnum;
//				user_set_velocity=sendnum;
//				new_flag=1;
//			}
//			USART_RX_STA=0;
//		}
//	 }	 	
	}
 
}	 
 
    // u8 res,ve,key=0;
	 // int cur;
   //u8 canbuf[8];
  //long  posi; 
//����whileѭ����
	//		key=Can_Receive_Msg(canbuf);
//		if(key)//CAN���յ�������
//		{			
//			LCD_Fill(60,270,130,310,WHITE);//���֮ǰ����ʾ
//			
// 			for(i=0;i<key;i++)
//			{									    
//				if(i<4)LCD_ShowxNum(60+i*32,270,canbuf[i],2,16,0X80);	//��ʾ����
//				else LCD_ShowxNum(60+(i-4)*32,290,canbuf[i],2,16,0X80);	//��ʾ����
// 			}
//			
//			res=Can_Send_Msg(canbuf,8);//����8���ֽ� 
//			if(res)LCD_ShowString(60+80,240,200,16,16,"Failed");		//��ʾ����ʧ��
//			else LCD_ShowString(60+80,240,200,16,16,"OK    ");	 		//��ʾ���ͳɹ�	
//			
//			
//			if(canbuf[0]==1)//ģʽ����
//				MODE=canbuf[1];
			

			
			
	//	}
	
	//��������  ��8�ֽ�   
//0 	02    1 ����ȷ�Ͻ�������  2  �����������
//1  	01    1 ����λ��ֵ��   2 �ٶ�ֵ  ����ֵ
//2	  01    1 ��   2 ��
//3	  00    �Ͷ�λ          ���﷢��ȥ����10���� ������λ������ʾ����16����
//4	  00    �ӡ��͵���
//5	  00
//6	  00
//7	  00
//	
//	
//	
//			if(MODE==0||MODE==3||MODE==5)
//			{
//			  canbuf[0]=2;
//				canbuf[1]=1;
//				if(Real_Position>=0)
//				  canbuf[2]=1;
//				else
//					canbuf[2]=2;
//				posi=labs(Real_Position);
//			  canbuf[3]=(u8)(posi%100);
//				posi/=100;
//				canbuf[4]=(u8)(posi%100);
//				posi/=100;
//				canbuf[5]=(u8)(posi%100);
//				posi/=100;
//				canbuf[6]=(u8)(posi%100);
//				canbuf[7]=0;
//				Can_Send_Msg(canbuf,8);
//			}
//			
//				if(MODE==1||MODE==4)
//			{
//			  canbuf[0]=2;
//				canbuf[1]=2;
//				if(now_velocity>=0)
//				  canbuf[2]=1;
//				else
//					canbuf[2]=2;
//				vel=labs(now_velocity);
//			  canbuf[3]=vel;
//			  cur=now_current;
//				canbuf[4]=cur%100;
//				cur/=100;
//				canbuf[5]=cur%100;
//				canbuf[6]=0;
//				canbuf[7]=0;
//				Can_Send_Msg(canbuf,8);
//			}
//			
//			
//			
//			


































