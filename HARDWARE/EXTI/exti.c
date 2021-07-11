#include "exti.h"
#include "delay.h"
#include "led.h"
#include "key.h"
#include "lcd.h"
#include "touch.h"
#include "text.h"
//#include "timer.h"
#include "audioplay.h"
#include "ft5206.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F429������
//�ⲿ�ж���������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2016/1/5
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	


#include <stdio.h>
 
char* Int2String(int num,char *str)//10?? 
{
    int i = 0;//????str 
		int j = 0;
    if(num<0)//??num???,?num?? 
    {
        num = -num;
        str[i++] = '-';
    } 
    //?? 
    do
    {
        str[i++] = num%10+48;//?num??? ??0~9?ASCII??48~57;??????0+48=48,ASCII?????'0' 
        num /= 10;//?????    
    }while(num);//num??0????
    
    str[i] = '\0';
    
    //????????? 
    
    if(str[0]=='-')//?????,?????? 
    {
        j = 1;//???????? 
        ++i;//?????,????????????1? 
    }
    //???? 
    for(;j<i/2;j++)
    {
        //???????? ????????????a+b??:a=a+b;b=a-b;a=a-b; 
        str[j] = str[j] + str[i-1-j];
        str[i-1-j] = str[j] - str[i-1-j];
        str[j] = str[j] - str[i-1-j];
    } 
    
    return str;//??????? 
}

extern	u8 curtainFlag;
extern	u8 lightFlag;
extern	u8 co2Flag;
extern	u8 doorFlag;
extern	u8 defence;
extern	u8 count;
extern  u8 key;		


//�ⲿ�жϳ�ʼ��
void EXTI_Init(void)
{
    GPIO_InitTypeDef GPIO_Initure;
	
	 __HAL_RCC_GPIOA_CLK_ENABLE();               //����GPIOAʱ��
    __HAL_RCC_GPIOC_CLK_ENABLE();               //����GPIOCʱ��
    __HAL_RCC_GPIOH_CLK_ENABLE();               //����GPIOHʱ��
    
	  //�������жϳ�ʼ��
	  GPIO_Initure.Pin=GPIO_PIN_7;            //PH7
    GPIO_Initure.Mode=GPIO_MODE_IT_RISING;  //??
    GPIO_Initure.Pull=GPIO_PULLUP;        //?????,????
    GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //??
    HAL_GPIO_Init(GPIOH,&GPIO_Initure);     //???
	
    HAL_NVIC_SetPriority(EXTI9_5_IRQn,2,0);       //??????2,?????0
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);             //?????0	

		//�����жϳ�ʼ��
		GPIO_Initure.Pin=GPIO_PIN_0;                //PA0
    GPIO_Initure.Mode=GPIO_MODE_IT_RISING;      //�����ش���
    GPIO_Initure.Pull=GPIO_PULLDOWN;
    HAL_GPIO_Init(GPIOA,&GPIO_Initure);
    
    GPIO_Initure.Pin=GPIO_PIN_13;               //PC13
    GPIO_Initure.Mode=GPIO_MODE_IT_FALLING;     //�½��ش�������
    GPIO_Initure.Pull=GPIO_PULLUP;
    HAL_GPIO_Init(GPIOC,&GPIO_Initure);
    
    GPIO_Initure.Pin=GPIO_PIN_2|GPIO_PIN_3;     //PH2,3
    HAL_GPIO_Init(GPIOH,&GPIO_Initure);
    
    //�ж���0-PA0
    HAL_NVIC_SetPriority(EXTI0_IRQn,2,0);       //��ռ���ȼ�Ϊ2�������ȼ�Ϊ0
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);             //ʹ���ж���0
    
    //�ж���2-PH2
    HAL_NVIC_SetPriority(EXTI2_IRQn,2,1);       //��ռ���ȼ�Ϊ2�������ȼ�Ϊ1
    HAL_NVIC_EnableIRQ(EXTI2_IRQn);             //ʹ���ж���2
    
    //�ж���3-PH3
    HAL_NVIC_SetPriority(EXTI3_IRQn,2,2);       //��ռ���ȼ�Ϊ2�������ȼ�Ϊ2
    HAL_NVIC_EnableIRQ(EXTI3_IRQn);             //ʹ���ж���2
    
    //�ж���13-PC13
    HAL_NVIC_SetPriority(EXTI15_10_IRQn,2,3);   //��ռ���ȼ�Ϊ2�������ȼ�Ϊ3
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);         //ʹ���ж���13  

}

//�������жϷ�����
void EXTI9_5_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_7);//??????????
}

//�����жϷ�����
void EXTI0_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);//�����жϴ����ú���
}

void EXTI2_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_2);//�����жϴ����ú���
}

void EXTI3_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_3);//�����жϴ����ú���
}

void EXTI15_10_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13);//�����жϴ����ú���
}

//�жϷ����������Ҫ��������
//��HAL�������е��ⲿ�жϷ�����������ô˺���
//GPIO_Pin:�ж����ź�
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	char string[16] = {0};
//  delay_ms(100);      //����
	switch(GPIO_Pin)
    {
				//�����жϴ���
				case GPIO_PIN_0:
					if(WK_UP==1) 
					{
					}
					break;
					
        case GPIO_PIN_2:
					if(KEY1==0)  //LED1��ת
					{
							lightFlag=!lightFlag;
					}
					break;
					
        case GPIO_PIN_3:
					if(KEY0==0)  //ͬʱ����LED0,LED1��ת 
					{
							curtainFlag=!curtainFlag;
					}
					break;

        case GPIO_PIN_13:
					if(KEY2==0)  
					{
							co2Flag=!co2Flag;
					}
					break;
						
				//�������жϴ���
        case GPIO_PIN_7:
					if(KEY_LCD==1)
					{
						//ʧ�ܵ��ж϶�ȡ
						//lcd_touch(GPIO_PIN_7);
						
						//˫��
						tp_dev.scan(0);
						while(tp_dev.x[0]==65535 && tp_dev.y[0]==65535)tp_dev.scan(0);
//						FT5206_Scan(0);
//						while(tp_dev.x[0]==65535 && tp_dev.y[0]==65535)FT5206_Scan(0);
						
//						//������������
//						Int2String(tp_dev.x[0],string);
//						Show_Str(30,50,200,16,string,16,0);
//						Int2String(tp_dev.y[0],string);
//						Show_Str(30,75,200,16,string,16,0);
					}
					break;
    }
}


