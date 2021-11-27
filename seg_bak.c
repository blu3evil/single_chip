#include <reg52.h>
#include <delay.h>

#define DEFAULT_FLICKER_EW 3		//东西方向黄灯默认闪烁时间
#define DEFAULT_FLICKER_SN 3		//南北方向黄灯默认闪烁时间


unsigned char data default_sec_seg = 20;
unsigned char data sec_seg = 20; 	//数码管倒计时
unsigned char data flicker_EW = 3; 	//东西方向黄灯闪烁时间
unsigned char data flicker_SN = 3; 	//南北方向黄灯闪烁时间

bit mode_chosen = 1;	//状态标志，置1为状态1，置0为状态2
bit mode = 1; 			//用于选择数码管显示模式，置1显示数码管倒计时，置0显示黄灯闪烁倒计时

sbit red_EW = P2^6;      //东西方向红灯
sbit yellow_EW = P2^5;	  //东风方向黄灯
sbit green_EW = P2^4;	  //东西方向绿灯

sbit red_SN = P2^3;       //南北方向红灯
sbit yellow_SN = P2^2;	  //南北方向黄灯
sbit green_SN = P2^1;	  //南北方向绿灯

//按键标志
sbit k1 = P1^5;			  //闪烁时间加
sbit k2 = P3^2;			  //闪烁时间减	(后续要改焊到其他端口避免占用RXD)
sbit k3 = P3^4;			  //数码管时间加
sbit k4 = P3^5;			  //数码管时间减
sbit k5 = P3^6;			  //切换当前方向
sbit k6	= P3^7;			  //确认更改以继续(待更改)

unsigned char recv_buf = 0;
unsigned char send_buf = 0;    //发送\接收缓冲区,一个字节长度

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

/*更新数码管状态函数，用于恢复暂停后数码管倒数状态*/
void Update(bit mode)
{
	if (mode)		//显示状态切换倒计时
	{
		P1 = 0xff;		//清零P1控制位	
		P0 = 0x00;		//清零P0数码管输入
		P1 = 0xfe;		//选择东西方向数码管十位
		P0 = seg_table[sec_seg/10];
		Delay1ms();

		P1 = 0xff;
		P0 = 0x00;
		P1 = 0xfd;		//选择东西方向数码管个位
		P0 = seg_table[sec_seg%10];
		Delay1ms();

		P1 = 0xff;
		P0 = 0x00;
		P1 = 0xfb;  	//选择南北方向数码管十位
		P0 = seg_table[sec_seg/10];
		Delay1ms();


		P1 = 0xff;
		P0 = 0x00;
		P1 = 0xf7;  	//选择南北方向数码管个位
		P0 = seg_table[sec_seg%10];
		Delay1ms();
	}
	else
	{
		P1 = 0xff;				//清零P1控制位	
		P0 = 0x00;				//清零P0数码管输入
		P1 = 0xfe;				//选择东西方向数码管十位
		P0 = seg_table[flicker_EW/10];
		Delay1ms();

		P1 = 0xff;
		P0 = 0x00;
		P1 = 0xfd;				//选择东西方向数码管个位
		P0 = seg_table[flicker_EW%10];
		Delay1ms();

		P1 = 0xff;
		P0 = 0x00;
		P1 = 0xfb;  	//选择南北方向数码管十位
		P0 = seg_table[flicker_SN/10];
		Delay1ms();


		P1 = 0xff;
		P0 = 0x00;
		P1 = 0xf7;  	//选择南北方向数码管个位
		P0 = seg_table[flicker_SN%10];
		Delay1ms();
	}
	
	
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
	mode = 0;
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
	mode = 0;
}

/*红绿灯码管倒计时增*/
void Seg_add(void)
{
	if (sec_seg == 99)
	{
		sec_seg = 0;	//形成0~99闭环
		return;
	}			
	sec_seg++;
	default_sec_seg++;
	mode = 1;
}

/*红绿灯数码管倒计时减*/
void Seg_sub(void)
{
	if (sec_seg == 0)
	{
		sec_seg = 99;
		return;
	}
	sec_seg--;
	default_sec_seg--;
	mode = 1;
}

void Led_Config(void)
{
	if (mode_chosen)	//状态1，南北方向黄灯将闪烁
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


/*定时器T0中断函数*/
void InterruptTimer0(void) interrupt 1
{
	/*黄灯控制部分*/
	static unsigned char count0 = 0;
	static unsigned char sec = 0;
	if (mode_chosen && sec_seg <= flicker_SN)		//南北方向黄灯闪烁(即将进入状态2)
	{
		if (count0 % 10 == 0)				//黄灯一秒闪烁一次,0.5s亮，0.5s灭
			yellow_SN = !yellow_SN;
	}
	 if (!mode_chosen && sec_seg <= flicker_EW)					//东西方向黄灯闪烁
	{
		if (count0 % 10 == 0)
			yellow_EW = !yellow_EW;
	}
	

	/*数码管部分*/
	/*黄灯倒计时结束切换状态1,2*/
	if (sec_seg == 0)		//黄灯倒计时归零
	{
		mode_chosen = !mode_chosen;					//切换状态
		sec_seg = default_sec_seg;
		Update(1);
		Led_Config();
	}
		
	
	if (count0 == 20)	//一秒时
	{
		count0 = 0;						//清空count0
		sec_seg--;
		sec++;
	}
	if (sec == 60)
	{
		sec = 0;
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
			mode = 0;
		}
	}

	if (k6 == 0)	//确认更改并继续
	{
		Delay10ms();
		if (k6 == 0)
		{
			TR0 = 1;	//启动定时器0，让时间开始流动
			mode = 1;
		}
	}

}






void Config_UART(unsigned int baud) 
{  
   SCON = 0x50; //串口模式一(SM0=0,SM1=1)
   TMOD &= 0x0F;  //清空T1控制位
   TMOD |= 0x20;  //T1设置为模式二,TH1存储重载数，TL1溢出时自动重载
   TH1 = 256 - (12000000/12/32)/baud; //12000000/12/baud相当于1s内机械周期个数*1bit数据传输所花费秒数(即1bit数据传输所花费的周期个数)
                                      //实际通信中，1bit数据将被捕获16次，单片机通过判断其中第7,8,9次电平状态来判定该数据为1或0，故取12000000/12/16/baud
                                      //我们人为意图取其中间时段，故再除以2
   TL1 = TH1;
   ET1 = 0; //禁止T1中断(供串口中断使用)
   ES = 1;
   TR1 = 1;
}



/*串口中断函数*/
void InterruptUART() interrupt 4
{
   if (RI) //接收到数据
   {
      RI = 0;   //手动清零
      recv_buf = SBUF;
      switch (recv_buf)
      {
         case 1:Pause(); Config_Timer0(); Flicker_add(); SBUF = recv_buf; break;
         case 2:Pause(); Config_Timer0(); Flicker_sub(); SBUF = recv_buf; break;
         case 3:Pause(); Config_Timer0(); Seg_add(); SBUF = recv_buf; break;
         case 4:Pause(); Config_Timer0(); Seg_sub(); SBUF = recv_buf; break;
         case 5:Pause(); Config_Timer0(); mode_chosen = !mode_chosen; SBUF = recv_buf; break;
         case 6:TR0 = 1; mode = 1;  SBUF = recv_buf; break;
         default: break;
      }
	  /*
      if (recv_buf | 0x10) //小时数
      hour = 0x3f & recv_buf;
      if (recv_buf | 0x01) //分钟数
      {
         min = 0x3f & recv_buf;
          Get_Time(hour,min);
      }
    */     
  }

   if (TI)     //发送完成
   {
      TI = 0;  
   }
}


void main()
{
	EA = 1;
	Config_Timer0();
	Config_UART(9600);
	Led_Config();
	TR0 = 1;
	while (1)
	{
		Key_Scan();
		Update(mode);
	}
}


