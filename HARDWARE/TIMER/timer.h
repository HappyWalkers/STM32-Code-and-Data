#ifndef _TIMER_H
#define _TIMER_H
#include "sys.h"
extern	u8 curtainFlag;
extern	u8 lightFlag;
extern	u8 co2Flag;
extern	u8 doorFlag;
extern	u8 defence;
extern	u8 count;
extern  u8 key;														//��ֵ
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F429������
//��ʱ����������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2016/1/6
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	
extern TIM_HandleTypeDef TIM3_Handler;      //��ʱ����� 

void TIM3_Init(u16 arr,u16 psc);
#endif

