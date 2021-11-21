#include <reg52.h>
#include <DELAY.h>

#define DEFAULT_SEG_EW 20		   	//东西方向数码管默认时间
#define DEFAULT_SEG_SN 23		   	//南北方向数码管默认时间
#define DEFAULT_FLICKER_EW 3		//东西方向黄灯默认闪烁时间
#define DEFAULT_FLICKER_SN 3		//南北方向黄灯默认闪烁时间

unsigned char data seg_EW = 20;    //东西方向数码管秒数
unsigned char data seg_SN = 2;    //南北方向数码管时间
unsigned char data flicker_EW = 3; //东西方向黄灯闪烁时间
unsigned char data flicker_SN = 3; //南北方向黄灯闪烁时间

bit mode_chosen = 1;	//目前选择的方向标志,置1为东西，置0为南北(除手动调整程序中会随当前黄灯亮起方向自动改变)

sbit red_EW = P2^6;      //东西方向红灯
sbit yellow_EW = P2^5;	  //东风方向黄灯
sbit green_EW = P2^4;	  //东西方向绿灯

sbit red_SN = P2^3;       //南北方向红灯
sbit yellow_SN = P2^2;	  //南北方向黄灯
sbit green_SN = P2^1;	  //南北方向绿灯

//按键标志
sbit k1 = P1^5;			  //闪烁时间加
sbit k2 = P3^1;			  //闪烁时间减	(后续要改焊到其他端口避免占用RXD)
sbit k3 = P3^4;			  //数码管时间加
sbit k4 = P3^5;			  //数码管时间减
sbit k5 = P3^6;			  //切换当前方向
sbit k6	= P3^7;			  //确认更改以继续

unsigned char seg_table[10]={  //共阴极数码管真值表
    0x3f,   //0,0b0011 1111 点亮a,b,c,d,e,f段,P0^0~P0^7由右到左排序(由低到高)
    0x06,   //1
    0x5b,   //2
    0x4f,   //3
    0x66,   //4
    0x6d,   //5
    0x7d,   //6 
    0x07,   //7
    0x7f,   //8
    0x6f   //9
};



/*配置T0定时器，用于数码管记时*/
void Config_Timer0(void)
{
	TMOD &= 0xF0;	//清零T0控制位
	TMOD |= 0x01;	//设置T0为模式一(16位计时模式)
	TH0 = 0x3C;   	//65536-(0.05*12000000/12)=15536 = 0x3CB0
	TL0 = 0xB0;		//50ms溢出一次，溢出20次则为一秒
	ET0 = 1;		//使能T0中断
}


/*停止函数，用于增加/减少时间时暂停进程*/
void Pause(void)
{
	TR0 = 0; //停止T0
}

/*闪烁时间增加函数*/
void Flicker_add(void)
{
	if (mode_chosen)  //东西方向
	{
		flicker_EW += 1;
	}
	else
	{
		flicker_SN += 1;
	}
}

/*闪烁时间减少函数*/
void Flicker_sub(void)
{
	if (mode_chosen)
	{
		flicker_EW -= 1;
	}
	else
	{
		flicker_SN -= 1;
	}
}

/*红绿灯码管倒计时增*/
void Seg_add(void)
{
	if (mode_chosen)
	{
		seg_EW += 1;
	}
	else
	{
		seg_SN += 1;
	}
}

/*红绿灯数码管倒计时减*/
void Seg_sub(void)
{
	if (mode_chosen)
	{
		seg_EW -= 1;
	}
	else
	{
		seg_SN -= 1;
	}
}

void Led_Config(void)
{
	if (mode_chosen)	//状态1，东西方向先黄灯
	{
		red_EW = 0;
		green_EW = 1;
		yellow_EW = 0;
		
		red_SN = 1;
		green_SN = 0;
		yellow_SN = 0;
	}
	else
	{
		red_EW = 1;
		green_EW = 0;
		yellow_EW = 0;
		
		red_SN = 0;
		green_SN = 1;
		yellow_SN = 0;
	}
}

/*更新数码管状态函数，用于恢复暂停后数码管倒数状态*/
void update()
{
	P1 = 0xff;		//清零P1控制位	
	P0 = 0x00;		//清零P0数码管输入
	P1 = 0xfe;		//选择东西方向数码管十位
	P0 = seg_table[seg_EW/10];
	Delay1ms();
	
	P1 = 0xff;
	P0 = 0x00;
	P1 = 0xfd;		//选择东西方向数码管个位
	P0 = seg_table[seg_EW%10];
	Delay1ms();
	
	P1 = 0xff;
	P0 = 0x00;
	P1 = 0xfb;  	//选择南北方向数码管十位
	P0 = seg_table[seg_SN/10];
	Delay1ms();
	
		
	P1 = 0xff;
	P0 = 0x00;
	P1 = 0xf7;  	//选择南北方向数码管个位
	P0 = seg_table[seg_SN%10];
	Delay1ms();
	
}

/*定时器T0中断函数*/
void InterruptTimer0(void) interrupt 1
{
	/*黄灯控制部分*/
	static unsigned char count0 = 0;
	if (seg_EW == seg_SN || (seg_EW == 0 && seg_SN != 0))		//东西方向黄灯开始闪烁
		if (count0 % 10 == 0)				//黄灯一秒闪烁一次,0.5s亮，0.5s灭
			yellow_EW = !yellow_EW;
	if (seg_EW == seg_SN || (seg_SN == 0 && seg_EW != 0))		//南北方向黄灯闪烁
		if (count0 % 10 == 0)
			yellow_SN = !yellow_SN;
	

	/*数码管部分*/
	/*黄灯倒计时结束切换状态1,2*/
	if (seg_EW == 0 && seg_SN == 0)		//黄灯倒计时归零
	{
		if (mode_chosen)				//东西方向黄灯倒计时归零
		{
			seg_EW = DEFAULT_SEG_SN;		//第一次启动默认东西方向先黄灯，
			seg_SN = DEFAULT_SEG_EW;		//故将东西方向与南北方向默认时间互换以切换黄灯反向
			mode_chosen = !mode_chosen;					//黄灯亮起方向变为南北
		}
		else
		{
			seg_EW = DEFAULT_SEG_EW;		
			seg_SN = DEFAULT_SEG_SN;
			mode_chosen = !mode_chosen;
		}
		update();
		Led_Config();
	}
	
	/*黄灯开始倒计时*/
	if (seg_EW == 0 && seg_SN !=0)		//东西方向黄灯倒计时等于南北方向红绿灯倒计时
	{
		seg_EW = seg_SN;
		update();
	}
	
	if (seg_SN == 0 && seg_EW !=0)		//南北方向黄灯倒计时等于东西方向红绿灯倒计时
	{
		seg_SN = seg_EW;
		update();
	}	
	

	if (count0 == 20)	//一秒时
	{
		count0 = 0;						//清空count0
		seg_EW -= 1;
		seg_SN -= 1;
		update();
	}
	count0++;
}

/*按键扫描函数*/
void Key_Scan(void)
{
	if (k1 != 1)	//k1按下,闪烁时间加
	{
		Delay10ms(); //延时消抖
		if (k1 != 1)  //确认按下
		{
			Pause();		  //暂停时间
			Config_Timer0(); //重新设置T0剩余时间
			Flicker_add();	 //闪烁时间加1
		}
			  
	}

	if (k2 == 0)	//闪烁时间减
	{
		Delay10ms();
		if  (k2 == 0)
		{
			Pause();
			Config_Timer0();
			Flicker_sub();  //闪烁时间减
		}
	}

	if (k3 == 0)	//当前方向数码管倒计时加
	{
		Delay10ms();
		if (k3 == 0)
		{
			Pause();
			Config_Timer0();
			Seg_add(); //数码管显示时间加一
		}
			
	}

	if (k4 == 0)	//当前方向数码管倒计时减
	{
		Delay10ms();
		if (k4 == 0)
		{
			Pause();
			Config_Timer0();
			Seg_sub();
		}
	}

	if (k5 == 0)	//切换当前选择方向
	{
		Delay10ms();
		if (k5 == 0)
		{
			Pause();
			Config_Timer0();
			mode_chosen = !mode_chosen;
		}
	}

	if (k6 == 0)	//确认更改并继续
	{
		Delay10ms();
		if (k6 == 0)
		{
			if (seg_EW > seg_SN)
				mode_chosen = 1;
			else
				mode_chosen = 0;
			TR0 = 1;	//启动定时器0，让时间开始流动
		}
	}

}



void main()
{
	EA = 1;
	Config_Timer0();
	Led_Config();
	TR0 = 1;
	while (1)
	{
		Key_Scan();
		update();
	}
}



