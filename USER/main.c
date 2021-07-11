#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "key.h"
#include "lcd.h"
//#include "timer.h"
#include "string.h"
#include "sdram.h"
#include "touch.h"				
#include "malloc.h"
#include "w25qxx.h"
#include "ff.h"
#include "exfuns.h"
#include "string.h"
#include "sdio_sdcard.h"
#include "fontupd.h"
#include "text.h"
#include "piclib.h"		
#include "math.h"
#include "wm8978.h"	 
#include "audioplay.h"

#include "exti.h"
#include "ft5206.h"

u8 curtainFlag;//0 means closed
u8 lightFlag;//0 means closed
u8 co2Flag;// 0 means safe
u8 doorFlag;
u8 defence;//0 means defence
u8 count;
u8 key;		

/************************************************
 ALIENTEK 阿波罗STM32F429开发板实验43
 图片显示实验-HAL库函数版
 技术支持：www.openedv.com
 淘宝店铺：http://eboard.taobao.com  
 关注微信公众平台微信号："正点原子"，免费获取STM32资料。
 广州市星翼电子科技有限公司  
 作者：正点原子 @ALIENTEK
************************************************/

//得到path路径下,目标文件的总个数
//path:路径		    
//返回值:总有效文件数
void Curtain_Init(){
	LED0=1;
	Show_Str(155,470,240,24,"窗帘状态：关闭",24,0); 	
	Show_Str(155,495,240,24,"电灯状态：关闭",24,0);
	Show_Str(155,520,240,24,"煤气安全：安全",24,0); 
	Show_Str(155,545,240,24,"门磁状态：关闭",24,0);
	Show_Str(155,570,240,24,"设防状态：设防",24,0); 		
}
u16 pic_get_tnum(u8 *path)
{	  
	u8 res;
	u16 rval=0;
 	DIR tdir;	 		//临时目录
	FILINFO *tfileinfo;	//临时文件信息	    			     
	tfileinfo=(FILINFO*)mymalloc(SRAMIN,sizeof(FILINFO));//申请内存
    res=f_opendir(&tdir,(const TCHAR*)path); 	//打开目录 
	if(res==FR_OK&&tfileinfo)
	{
		while(1)//查询总的有效文件数
		{
	        res=f_readdir(&tdir,tfileinfo);       		//读取目录下的一个文件  	 
	        if(res!=FR_OK||tfileinfo->fname[0]==0)break;//错误了/到末尾了,退出	 		 
			res=f_typetell((u8*)tfileinfo->fname);
			if((res&0XF0)==0X50)//取高四位,看看是不是图片文件	
			{
				rval++;//有效文件数增加1
			}	    
		}  
	}  
	myfree(SRAMIN,tfileinfo);//释放内存
	return rval;
}

int main(void)
{	
	u8 res;
 	DIR picdir;	 											//图片目录
	FILINFO *picfileinfo;							//文件信息 
	u8 *pname;												//带路径的文件名
	u16 totpicnum; 										//图片文件总数
	u16 curindex;											//图片当前索引
	
	u16 temp;
	u32 *picoffsettbl;								//图片文件offset索引表 

  
	HAL_Init();                     	//初始化HAL库   
	Stm32_Clock_Init(360,25,2,8);   	//设置时钟,180Mhz
	delay_init(180);                	//初始化延时函数
	uart_init(115200);              	//初始化USART
	LED_Init();												//初始化与LED连接的硬件接口
	
	
	
	SDRAM_Init();											//初始化SDRAM 
	LCD_Init();												//初始化LCD
	KEY_Init();												//初始化按键
	//TIM3_Init(50-1,9000-1);
	FT5206_Init();
	
	W25QXX_Init();										//初始化W25Q256
	tp_dev.init();	
	W25QXX_Init();				    				//初始化W25Q256
  WM8978_Init();				   			 		//初始化WM8978
	WM8978_HPvol_Set(40,40);	    		//耳机音量设置
	WM8978_SPKvol_Set(40);		    		//喇叭音量设置
 	my_mem_init(SRAMIN);		  				//初始化内部内存池
	my_mem_init(SRAMEX);							//初始化外部内存池
	my_mem_init(SRAMCCM);							//初始化CCM内存池 
	exfuns_init();										//为fatfs相关变量申请内存  
 	f_mount(fs[0],"0:",1); 						//挂载SD卡 
 	f_mount(fs[1],"1:",1); 						//挂载FLASH.
 	f_mount(fs[2],"2:",1); 						//挂载NAND FLASH.
	POINT_COLOR=BLUE;      
	
	EXTI_Init();                    //外部中断初始化
	
	while(font_init()) 																			//检查字库
	{	    
		LCD_ShowString(30,50,200,16,16,"Font Error!");
		delay_ms(200);				  
		LCD_Fill(30,50,240,66,WHITE);													//清除显示	     
		delay_ms(200);				  
	}  	 
 	Show_Str(30,50,200,16,"阿波罗STM32F4/F7开发板",16,0);				    	 
	Show_Str(30,70,200,16,"图片显示程序",16,0);				    	 
	Show_Str(30,90,200,16,"KEY0:NEXT KEY2:PREV",16,0);				    	 
	Show_Str(30,110,200,16,"KEY_UP:PAUSE",16,0);				    	 
	Show_Str(30,130,200,16,"正点原子@ALIENTEK",16,0);				    	 
	Show_Str(30,150,200,16,"2016年1月7日",16,0);
 	while(f_opendir(&picdir,"0:/PICTURE"))									//打开图片文件夹
 	{	    
		Show_Str(30,170,240,16,"PICTURE文件夹错误!",16,0);
		delay_ms(200);				  
		LCD_Fill(30,170,240,186,WHITE);												//清除显示	     
		delay_ms(200);				  
	}  
	totpicnum=pic_get_tnum("0:/PICTURE"); 									//得到总有效文件数
  	while(totpicnum==NULL)																//图片文件为0		
 	{	    
		Show_Str(30,170,240,16,"没有图片文件!",16,0);
		delay_ms(200);				  
		LCD_Fill(30,170,240,186,WHITE);												//清除显示	     
		delay_ms(200);				  
	} 
	picfileinfo=(FILINFO*)mymalloc(SRAMIN,sizeof(FILINFO));	//申请内存
 	pname=mymalloc(SRAMIN,_MAX_LFN*2+1);										//为带路径的文件名分配内存
 	picoffsettbl=mymalloc(SRAMIN,4*totpicnum);							//申请4*totpicnum个字节的内存,用于存放图片索引
 	while(!picfileinfo||!pname||!picoffsettbl)							//内存分配出错
 	{	    	
		Show_Str(30,170,240,16,"内存分配失败!",16,0);
		delay_ms(200);				  
		LCD_Fill(30,170,240,186,WHITE);												//清除显示	     
		delay_ms(200);				  
	}  	
																													//记录索引
    res=f_opendir(&picdir,"0:/PICTURE"); 									//打开目录
	if(res==FR_OK)
	{
		curindex=0;																						//当前索引为0
		while(1)																							//全部查询一遍
		{
			temp=picdir.dptr;																		//记录当前dptr偏移
	        res=f_readdir(&picdir,picfileinfo);       			//读取目录下的一个文件
	        if(res!=FR_OK||picfileinfo->fname[0]==0)break;	//错误了/到末尾了,退出	 	 
			res=f_typetell((u8*)picfileinfo->fname);	
			if((res&0XF0)==0X50)																//取高四位,看看是不是图片文件	
			{
				picoffsettbl[curindex]=temp;											//记录索引
				curindex++;
			}	    
		} 
	}   
	Show_Str(30,170,240,16,"开始显示...",16,0); 
	delay_ms(1500);
	piclib_init();																					//初始化画图	   	   
	curindex=0;																							//从0开始显示
   	res=f_opendir(&picdir,(const TCHAR*)"0:/PICTURE"); 		//打开目录
	
	
	while(res==FR_OK)																				//打开成功
	{	
		audio_play();  //开启成功，响一声
		dir_sdi(&picdir,picoffsettbl[curindex]);							//改变当前目录索引	   
        res=f_readdir(&picdir,picfileinfo);       				//读取目录下的一个文件
        if(res!=FR_OK||picfileinfo->fname[0]==0)break;		//错误了/到末尾了,退出
		strcpy((char*)pname,"0:/PICTURE/");										//复制路径(目录)
		strcat((char*)pname,(const char*)picfileinfo->fname);	//将文件名接在后面
 		LCD_Clear(BLACK);
 		ai_load_picfile(pname,0,0,lcddev.width,lcddev.height,0);
																													//显示图片    
		//Show_Str(2,2,lcddev.width,16,pname,16,1); 					//显示图片名字

		Curtain_Init();
		curtainFlag=0;
		lightFlag=0;
		co2Flag=0;
		defence=0;
		//count=0;
		while(1) 
		{
			//tp_dev.scan(0);
			if(tp_dev.x[0]>56&&tp_dev.x[0]<223&&tp_dev.y[0]>62&&tp_dev.y[0]<125){//按到窗帘打开按钮
				Show_Str(155,470,240,24,"窗帘状态：打开",24,0); 
				if(curtainFlag==0)curtainFlag=!curtainFlag;
			}
			if(tp_dev.x[0]>251&&tp_dev.x[0]<418&&tp_dev.y[0]>62&&tp_dev.y[0]<125){//按到窗帘关闭按钮
				Show_Str(155,470,240,24,"窗帘状态：关闭",24,0); 
				if(curtainFlag!=0)curtainFlag=!curtainFlag;
			}
			if(tp_dev.x[0]>56&&tp_dev.x[0]<223&&tp_dev.y[0]>170&&tp_dev.y[0]<230){
				Show_Str(155,495,240,24,"电灯状态：打开",24,0); 
				if(lightFlag==0)lightFlag=!lightFlag;
			}
			if(tp_dev.x[0]>251&&tp_dev.x[0]<418&&tp_dev.y[0]>170&&tp_dev.y[0]<230){
				Show_Str(155,495,240,24,"电灯状态：关闭",24,0); 
				if(lightFlag!=0)lightFlag=!lightFlag;
			}
			if(tp_dev.x[0]>56&&tp_dev.x[0]<223&&tp_dev.y[0]>275&&tp_dev.y[0]<335){
				Show_Str(155,570,240,24,"设防状态：设防",24,0);
				if(defence!=0)defence=!defence;
			}
			if(tp_dev.x[0]>251&&tp_dev.x[0]<418&&tp_dev.y[0]>275&&tp_dev.y[0]<335){
				Show_Str(155,570,240,24,"设防状态：撤防",24,0); 
				if(defence==0)defence=!defence;
			}
			

				if(curtainFlag==0){
					Show_Str(155,470,240,24,"窗帘状态：关闭",24,0);
				}else{
					Show_Str(155,470,240,24,"窗帘状态：打开",24,0);
					}
				if(lightFlag==0){
					Show_Str(155,495,240,24,"电灯状态：关闭",24,0);
				}else{
					Show_Str(155,495,240,24,"电灯状态：打开",24,0);
				}
				if(co2Flag==0){
					Show_Str(155,520,240,24,"煤气安全：安全",24,0);
				}else{
					Show_Str(155,520,240,24,"煤气安全：危险",24,0);
					if(curtainFlag==0)curtainFlag=!curtainFlag;//关闭就打开
					Show_Str(155,470,240,24,"窗帘状态：打开",24,0);
				}
			if(key==WKUP_PRES){
				LED1=!LED1; 							//暂停的时候LED1亮.  
				delay_ms(1000); 
			}
			
			if( (defence==0&&co2Flag==1) || (defence==0&&curtainFlag==1) ){//警戒状态且煤气危险，或窗帘打开时，报警；当按键破坏条件后，停止播放警戒
				audio_play();
			}
			
		}					    
		res=0;  
	} 							    
	myfree(SRAMIN,picfileinfo);			//释放内存						   		    
	myfree(SRAMIN,pname);						//释放内存			    
	myfree(SRAMIN,picoffsettbl);		//释放内存

}



