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
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F429开发板
//外部中断驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2016/1/5
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
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


//外部中断初始化
void EXTI_Init(void)
{
    GPIO_InitTypeDef GPIO_Initure;
	
	 __HAL_RCC_GPIOA_CLK_ENABLE();               //开启GPIOA时钟
    __HAL_RCC_GPIOC_CLK_ENABLE();               //开启GPIOC时钟
    __HAL_RCC_GPIOH_CLK_ENABLE();               //开启GPIOH时钟
    
	  //触摸屏中断初始化
	  GPIO_Initure.Pin=GPIO_PIN_7;            //PH7
    GPIO_Initure.Mode=GPIO_MODE_IT_RISING;  //??
    GPIO_Initure.Pull=GPIO_PULLUP;        //?????,????
    GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //??
    HAL_GPIO_Init(GPIOH,&GPIO_Initure);     //???
	
    HAL_NVIC_SetPriority(EXTI9_5_IRQn,2,0);       //??????2,?????0
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);             //?????0	

		//按键中断初始化
		GPIO_Initure.Pin=GPIO_PIN_0;                //PA0
    GPIO_Initure.Mode=GPIO_MODE_IT_RISING;      //上升沿触发
    GPIO_Initure.Pull=GPIO_PULLDOWN;
    HAL_GPIO_Init(GPIOA,&GPIO_Initure);
    
    GPIO_Initure.Pin=GPIO_PIN_13;               //PC13
    GPIO_Initure.Mode=GPIO_MODE_IT_FALLING;     //下降沿触发、、
    GPIO_Initure.Pull=GPIO_PULLUP;
    HAL_GPIO_Init(GPIOC,&GPIO_Initure);
    
    GPIO_Initure.Pin=GPIO_PIN_2|GPIO_PIN_3;     //PH2,3
    HAL_GPIO_Init(GPIOH,&GPIO_Initure);
    
    //中断线0-PA0
    HAL_NVIC_SetPriority(EXTI0_IRQn,2,0);       //抢占优先级为2，子优先级为0
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);             //使能中断线0
    
    //中断线2-PH2
    HAL_NVIC_SetPriority(EXTI2_IRQn,2,1);       //抢占优先级为2，子优先级为1
    HAL_NVIC_EnableIRQ(EXTI2_IRQn);             //使能中断线2
    
    //中断线3-PH3
    HAL_NVIC_SetPriority(EXTI3_IRQn,2,2);       //抢占优先级为2，子优先级为2
    HAL_NVIC_EnableIRQ(EXTI3_IRQn);             //使能中断线2
    
    //中断线13-PC13
    HAL_NVIC_SetPriority(EXTI15_10_IRQn,2,3);   //抢占优先级为2，子优先级为3
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);         //使能中断线13  

}

//触摸屏中断服务函数
void EXTI9_5_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_7);//??????????
}

//按键中断服务函数
void EXTI0_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);//调用中断处理公用函数
}

void EXTI2_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_2);//调用中断处理公用函数
}

void EXTI3_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_3);//调用中断处理公用函数
}

void EXTI15_10_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13);//调用中断处理公用函数
}

//中断服务程序中需要做的事情
//在HAL库中所有的外部中断服务函数都会调用此函数
//GPIO_Pin:中断引脚号
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	char string[16] = {0};
//  delay_ms(100);      //消抖
	switch(GPIO_Pin)
    {
				//按键中断处理
				case GPIO_PIN_0:
					if(WK_UP==1) 
					{
					}
					break;
					
        case GPIO_PIN_2:
					if(KEY1==0)  //LED1翻转
					{
							lightFlag=!lightFlag;
					}
					break;
					
        case GPIO_PIN_3:
					if(KEY0==0)  //同时控制LED0,LED1翻转 
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
						
				//触摸屏中断处理
        case GPIO_PIN_7:
					if(KEY_LCD==1)
					{
						//失败的中断读取
						//lcd_touch(GPIO_PIN_7);
						
						//双击
						tp_dev.scan(0);
						while(tp_dev.x[0]==65535 && tp_dev.y[0]==65535)tp_dev.scan(0);
//						FT5206_Scan(0);
//						while(tp_dev.x[0]==65535 && tp_dev.y[0]==65535)FT5206_Scan(0);
						
//						//输出点击的坐标
//						Int2String(tp_dev.x[0],string);
//						Show_Str(30,50,200,16,string,16,0);
//						Int2String(tp_dev.y[0],string);
//						Show_Str(30,75,200,16,string,16,0);
					}
					break;
    }
}


