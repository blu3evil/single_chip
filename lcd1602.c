#include <reg52.h>
#define LCD_DB P0   //lcd数据引脚

sbit LCD_RS = P1^6;
sbit LCD_RW = P1^7;
sbit LCD_EN = P1^4;         //lcd模块对应引脚


void LcdWaitReady(void)
{
    unsigned char stat;
    LCD_DB = 0xff;
    LCD_RS = 0;     //RS置高电平为数据模式，低电平为命令模式
    LCD_RW = 1;     //RW置高电平为读，低电平为写
    do 
    {
        LCD_EN = 1;
        stat = LCD_DB;  //读取状态字
        LCD_EN = 0;     //释放使能防止干扰数码管
    }while (stat & 0x80)    //bit7等于1说明液晶正忙，重复检测直至等于0为止
}

/*向液晶模块写入命令*/
void LcdLoadCmd(unsigned char cmd)
{
    LcdWaitReady();
    LCD_RS = 0;
    LCD_RW = 0;
    LCD_DB = cmd;
    LCD_EN = 1;
    LCD_EN = 0;
}

/*向液晶模块写入数据*/
void LcdLoadData(unsigned char dat)
{
    LcdWaitReady();
    LCD_RS = 1;
    LCD_RW = 0;
    LCD_DB = dat;
    LCD_EN = 1;
    LCD_EN = 0;
}

/*设置起始坐标*/
void LcdConfigCursor(unsigned char x,unsigned char y)
{
    unsigned char addr;
    if (y == 0)                 //第一行
        addr = 0x00 + x;        //第一行从0x00开始
    else
        addr = 0x40 + x;        //第一行从0x40开始
    LcdLoadCmd(addr | 0x80);     //RAM地址设置指令第一位必须为1,后七位为LCD RAM地址
}

/*字符串显示函数*/
void LcdLoadStr(unsigned x,unsigned y,unsigned char *str)
{
    LcdConfigCursor(x,y);   //设置起始坐标
    while (*str != '\0')    //逐个字符读入
        LcdLoadData(*str++);
}

/*初始化函数*/
void Init_Lcd(void)
{
    LcdLoadCmd(0x38);       //显示模式设置，固定步骤
    LcdLoadCmd(0x0c);       //0b00001DCB  D置1开显示,C置1显示光标,B置1光标闪烁,0x0c为开显示无光标
    LcdLoadCmd(0x06);       //0b000001NS  N置1读写字符后指针自动加一，S置1指针自动减一，达到移动光标的效果
    LcdLoadCmd(0x01);       //清屏指令
}

/*通过蓝牙获取网络时间，只获取一次，然后将值载入到定时函数中自动更新*/
void Get_Time(unsigned char hour,unsigned char min)
{
    //unsigned char *str;
    unsigned char str_time[6];
    str_time[0] = hour / 10 + '0';
    str_time[1] = hour % 10 + '0';
    str_time[2] = ':';
    str_time[3] = min / 10 + '0';
    str_time[4] = min % 10 + '0';
    LcdLoadStr(str_time);    
}