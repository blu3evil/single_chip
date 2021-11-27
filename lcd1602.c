#include <reg52.h>
#define LCD_DB P0   //lcd��������

sbit LCD_RS = P1^6;
sbit LCD_RW = P1^7;
sbit LCD_EN = P1^4;         //lcdģ���Ӧ����


void LcdWaitReady(void)
{
    unsigned char stat;
    LCD_DB = 0xff;
    LCD_RS = 0;     //RS�øߵ�ƽΪ����ģʽ���͵�ƽΪ����ģʽ
    LCD_RW = 1;     //RW�øߵ�ƽΪ�����͵�ƽΪд
    do 
    {
        LCD_EN = 1;
        stat = LCD_DB;  //��ȡ״̬��
        LCD_EN = 0;     //�ͷ�ʹ�ܷ�ֹ���������
    }while (stat & 0x80)    //bit7����1˵��Һ����æ���ظ����ֱ������0Ϊֹ
}

/*��Һ��ģ��д������*/
void LcdLoadCmd(unsigned char cmd)
{
    LcdWaitReady();
    LCD_RS = 0;
    LCD_RW = 0;
    LCD_DB = cmd;
    LCD_EN = 1;
    LCD_EN = 0;
}

/*��Һ��ģ��д������*/
void LcdLoadData(unsigned char dat)
{
    LcdWaitReady();
    LCD_RS = 1;
    LCD_RW = 0;
    LCD_DB = dat;
    LCD_EN = 1;
    LCD_EN = 0;
}

/*������ʼ����*/
void LcdConfigCursor(unsigned char x,unsigned char y)
{
    unsigned char addr;
    if (y == 0)                 //��һ��
        addr = 0x00 + x;        //��һ�д�0x00��ʼ
    else
        addr = 0x40 + x;        //��һ�д�0x40��ʼ
    LcdLoadCmd(addr | 0x80);     //RAM��ַ����ָ���һλ����Ϊ1,����λΪLCD RAM��ַ
}

/*�ַ�����ʾ����*/
void LcdLoadStr(unsigned x,unsigned y,unsigned char *str)
{
    LcdConfigCursor(x,y);   //������ʼ����
    while (*str != '\0')    //����ַ�����
        LcdLoadData(*str++);
}

/*��ʼ������*/
void Init_Lcd(void)
{
    LcdLoadCmd(0x38);       //��ʾģʽ���ã��̶�����
    LcdLoadCmd(0x0c);       //0b00001DCB  D��1����ʾ,C��1��ʾ���,B��1�����˸,0x0cΪ����ʾ�޹��
    LcdLoadCmd(0x06);       //0b000001NS  N��1��д�ַ���ָ���Զ���һ��S��1ָ���Զ���һ���ﵽ�ƶ�����Ч��
    LcdLoadCmd(0x01);       //����ָ��
}

/*ͨ��������ȡ����ʱ�䣬ֻ��ȡһ�Σ�Ȼ��ֵ���뵽��ʱ�������Զ�����*/
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