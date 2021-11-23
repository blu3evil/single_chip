/*蓝牙通信框架 v1.0 */ 
/*后续可能改为头文件使用，或另一个源程序*/

/*
要求通过蓝牙能改变黄灯闪烁时间与数码管倒计时
通过复用segment_displayer中的函数实现
思路:规定收到不同数据时分别调用不同函数
规定:0x01为黄灯闪烁时间加(步幅为1)
     0x02为黄灯闪烁时间减
     0x03为数码管时间加
     0x04为数码管时间减
     0x05为改变选择方向
     0x06为确认改变并继续
     */


#include <reg52.h>

sbit PIN_TXD = P3^1;
sbit PIN_RXD = P3^0;   //RXD,TXD电平状态



unsigned char recv_buf = 0;
unsigned char send_buf = 0;    //发送\接收缓冲区,一个字节长度

/*配置波特率函数*/
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
      swich (recv_buf)
      {
         case 0x01:Pause(); Config_Timer0(); Flicker_add(); break;
         case 0x02:Pause(); Config_Timer0(); Flicker_sub(); break;
         case 0x03:Pause(); Config_Timer0(); Seg_add();  break;
         case 0x04:Pause(); Config_Timer0(); Seg_sub();  break;
         case 0x05:Pause(); Config_Timer0(); mode_chosen = !mode_chosen; break;
         case 0x06:TR0 = 1; break;
         default: break;
      }    
   }

   if (TI)     //发送完成
   {
      TI = 0;  
   }
}
