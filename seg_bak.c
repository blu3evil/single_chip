#include <reg52.h>
#include <delay.h>

#define DEFAULT_FLICKER_EW 3		//��������Ƶ�Ĭ����˸ʱ��
#define DEFAULT_FLICKER_SN 3		//�ϱ�����Ƶ�Ĭ����˸ʱ��


unsigned char data default_sec_seg = 20;
unsigned char data sec_seg = 20; 	//����ܵ���ʱ
unsigned char data flicker_EW = 3; 	//��������Ƶ���˸ʱ��
unsigned char data flicker_SN = 3; 	//�ϱ�����Ƶ���˸ʱ��

bit mode_chosen = 1;	//״̬��־����1Ϊ״̬1����0Ϊ״̬2
bit mode = 1; 			//����ѡ���������ʾģʽ����1��ʾ����ܵ���ʱ����0��ʾ�Ƶ���˸����ʱ

sbit red_EW = P2^6;      //����������
sbit yellow_EW = P2^5;	  //���緽��Ƶ�
sbit green_EW = P2^4;	  //���������̵�

sbit red_SN = P2^3;       //�ϱ�������
sbit yellow_SN = P2^2;	  //�ϱ�����Ƶ�
sbit green_SN = P2^1;	  //�ϱ������̵�

//������־
sbit k1 = P1^5;			  //��˸ʱ���
sbit k2 = P3^2;			  //��˸ʱ���	(����Ҫ�ĺ��������˿ڱ���ռ��RXD)
sbit k3 = P3^4;			  //�����ʱ���
sbit k4 = P3^5;			  //�����ʱ���
sbit k5 = P3^6;			  //�л���ǰ����
sbit k6	= P3^7;			  //ȷ�ϸ����Լ���(������)

unsigned char recv_buf = 0;
unsigned char send_buf = 0;    //����\���ջ�����,һ���ֽڳ���

unsigned char seg_table[10]={  //�������������ֵ��
    0x3f,   //0,0b0011 1111 ����a,b,c,d,e,f��,P0^0~P0^7���ҵ�������(�ɵ͵���)
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



/*����T0��ʱ������������ܼ�ʱ*/
void Config_Timer0(void)
{
	TMOD &= 0xF0;	//����T0����λ
	TMOD |= 0x01;	//����T0Ϊģʽһ(16λ��ʱģʽ)
	TH0 = 0x3C;   	//65536-(0.05*12000000/12)=15536 = 0x3CB0
	TL0 = 0xB0;		//50ms���һ�Σ����20����Ϊһ��
	ET0 = 1;		//ʹ��T0�ж�
}

/*���������״̬���������ڻָ���ͣ������ܵ���״̬*/
void Update(bit mode)
{
	if (mode)		//��ʾ״̬�л�����ʱ
	{
		P1 = 0xff;		//����P1����λ	
		P0 = 0x00;		//����P0���������
		P1 = 0xfe;		//ѡ�������������ʮλ
		P0 = seg_table[sec_seg/10];
		Delay1ms();

		P1 = 0xff;
		P0 = 0x00;
		P1 = 0xfd;		//ѡ������������ܸ�λ
		P0 = seg_table[sec_seg%10];
		Delay1ms();

		P1 = 0xff;
		P0 = 0x00;
		P1 = 0xfb;  	//ѡ���ϱ����������ʮλ
		P0 = seg_table[sec_seg/10];
		Delay1ms();


		P1 = 0xff;
		P0 = 0x00;
		P1 = 0xf7;  	//ѡ���ϱ���������ܸ�λ
		P0 = seg_table[sec_seg%10];
		Delay1ms();
	}
	else
	{
		P1 = 0xff;				//����P1����λ	
		P0 = 0x00;				//����P0���������
		P1 = 0xfe;				//ѡ�������������ʮλ
		P0 = seg_table[flicker_EW/10];
		Delay1ms();

		P1 = 0xff;
		P0 = 0x00;
		P1 = 0xfd;				//ѡ������������ܸ�λ
		P0 = seg_table[flicker_EW%10];
		Delay1ms();

		P1 = 0xff;
		P0 = 0x00;
		P1 = 0xfb;  	//ѡ���ϱ����������ʮλ
		P0 = seg_table[flicker_SN/10];
		Delay1ms();


		P1 = 0xff;
		P0 = 0x00;
		P1 = 0xf7;  	//ѡ���ϱ���������ܸ�λ
		P0 = seg_table[flicker_SN%10];
		Delay1ms();
	}
	
	
}

/*ֹͣ��������������/����ʱ��ʱ��ͣ����*/
void Pause(void)
{
	TR0 = 0; //ֹͣT0
}

/*��˸ʱ�����Ӻ���*/
void Flicker_add(void)
{
	if (mode_chosen)  //��������
	{
		flicker_EW += 1;
	}
	else
	{
		flicker_SN += 1;
	}
	mode = 0;
}

/*��˸ʱ����ٺ���*/
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

/*���̵���ܵ���ʱ��*/
void Seg_add(void)
{
	if (sec_seg == 99)
	{
		sec_seg = 0;	//�γ�0~99�ջ�
		return;
	}			
	sec_seg++;
	default_sec_seg++;
	mode = 1;
}

/*���̵�����ܵ���ʱ��*/
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
	if (mode_chosen)	//״̬1���ϱ�����Ƶƽ���˸
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


/*��ʱ��T0�жϺ���*/
void InterruptTimer0(void) interrupt 1
{
	/*�Ƶƿ��Ʋ���*/
	static unsigned char count0 = 0;
	static unsigned char sec = 0;
	if (mode_chosen && sec_seg <= flicker_SN)		//�ϱ�����Ƶ���˸(��������״̬2)
	{
		if (count0 % 10 == 0)				//�Ƶ�һ����˸һ��,0.5s����0.5s��
			yellow_SN = !yellow_SN;
	}
	 if (!mode_chosen && sec_seg <= flicker_EW)					//��������Ƶ���˸
	{
		if (count0 % 10 == 0)
			yellow_EW = !yellow_EW;
	}
	

	/*����ܲ���*/
	/*�ƵƵ���ʱ�����л�״̬1,2*/
	if (sec_seg == 0)		//�ƵƵ���ʱ����
	{
		mode_chosen = !mode_chosen;					//�л�״̬
		sec_seg = default_sec_seg;
		Update(1);
		Led_Config();
	}
		
	
	if (count0 == 20)	//һ��ʱ
	{
		count0 = 0;						//���count0
		sec_seg--;
		sec++;
	}
	if (sec == 60)
	{
		sec = 0;
	}
	count0++;
}

/*����ɨ�躯��*/
void Key_Scan(void)
{
	if (k1 != 1)	//k1����,��˸ʱ���
	{
		Delay10ms(); //��ʱ����
		if (k1 != 1)  //ȷ�ϰ���
		{
			Pause();		  //��ͣʱ��
			Config_Timer0(); //��������T0ʣ��ʱ��
			Flicker_add();	 //��˸ʱ���1
		}
			  
	}

	if (k2 == 0)	//��˸ʱ���
	{
		Delay10ms();
		if  (k2 == 0)
		{
			Pause();
			Config_Timer0();
			Flicker_sub();  //��˸ʱ���
		}
	}

	if (k3 == 0)	//��ǰ��������ܵ���ʱ��
	{
		Delay10ms();
		if (k3 == 0)
		{
			Pause();
			Config_Timer0();
			Seg_add(); //�������ʾʱ���һ
		}
			
	}

	if (k4 == 0)	//��ǰ��������ܵ���ʱ��
	{
		Delay10ms();
		if (k4 == 0)
		{
			Pause();
			Config_Timer0();
			Seg_sub();
		}
	}

	if (k5 == 0)	//�л���ǰѡ����
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

	if (k6 == 0)	//ȷ�ϸ��Ĳ�����
	{
		Delay10ms();
		if (k6 == 0)
		{
			TR0 = 1;	//������ʱ��0����ʱ�俪ʼ����
			mode = 1;
		}
	}

}






void Config_UART(unsigned int baud) 
{  
   SCON = 0x50; //����ģʽһ(SM0=0,SM1=1)
   TMOD &= 0x0F;  //���T1����λ
   TMOD |= 0x20;  //T1����Ϊģʽ��,TH1�洢��������TL1���ʱ�Զ�����
   TH1 = 256 - (12000000/12/32)/baud; //12000000/12/baud�൱��1s�ڻ�е���ڸ���*1bit���ݴ�������������(��1bit���ݴ��������ѵ����ڸ���)
                                      //ʵ��ͨ���У�1bit���ݽ�������16�Σ���Ƭ��ͨ���ж����е�7,8,9�ε�ƽ״̬���ж�������Ϊ1��0����ȡ12000000/12/16/baud
                                      //������Ϊ��ͼȡ���м�ʱ�Σ����ٳ���2
   TL1 = TH1;
   ET1 = 0; //��ֹT1�ж�(�������ж�ʹ��)
   ES = 1;
   TR1 = 1;
}



/*�����жϺ���*/
void InterruptUART() interrupt 4
{
   if (RI) //���յ�����
   {
      RI = 0;   //�ֶ�����
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
      if (recv_buf | 0x10) //Сʱ��
      hour = 0x3f & recv_buf;
      if (recv_buf | 0x01) //������
      {
         min = 0x3f & recv_buf;
          Get_Time(hour,min);
      }
    */     
  }

   if (TI)     //�������
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


