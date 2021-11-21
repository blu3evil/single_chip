#include <reg52.h>
#include <DELAY.h>

#define DEFAULT_SEG_EW 20		   	//�������������Ĭ��ʱ��
#define DEFAULT_SEG_SN 23		   	//�ϱ����������Ĭ��ʱ��
#define DEFAULT_FLICKER_EW 3		//��������Ƶ�Ĭ����˸ʱ��
#define DEFAULT_FLICKER_SN 3		//�ϱ�����Ƶ�Ĭ����˸ʱ��

unsigned char data seg_EW = 20;    //�����������������
unsigned char data seg_SN = 2;    //�ϱ����������ʱ��
unsigned char data flicker_EW = 3; //��������Ƶ���˸ʱ��
unsigned char data flicker_SN = 3; //�ϱ�����Ƶ���˸ʱ��

bit mode_chosen = 1;	//Ŀǰѡ��ķ����־,��1Ϊ��������0Ϊ�ϱ�(���ֶ����������л��浱ǰ�Ƶ��������Զ��ı�)

sbit red_EW = P2^6;      //����������
sbit yellow_EW = P2^5;	  //���緽��Ƶ�
sbit green_EW = P2^4;	  //���������̵�

sbit red_SN = P2^3;       //�ϱ�������
sbit yellow_SN = P2^2;	  //�ϱ�����Ƶ�
sbit green_SN = P2^1;	  //�ϱ������̵�

//������־
sbit k1 = P1^5;			  //��˸ʱ���
sbit k2 = P3^1;			  //��˸ʱ���	(����Ҫ�ĺ��������˿ڱ���ռ��RXD)
sbit k3 = P3^4;			  //�����ʱ���
sbit k4 = P3^5;			  //�����ʱ���
sbit k5 = P3^6;			  //�л���ǰ����
sbit k6	= P3^7;			  //ȷ�ϸ����Լ���

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
}

/*���̵���ܵ���ʱ��*/
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

/*���̵�����ܵ���ʱ��*/
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
	if (mode_chosen)	//״̬1�����������ȻƵ�
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

/*���������״̬���������ڻָ���ͣ������ܵ���״̬*/
void update()
{
	P1 = 0xff;		//����P1����λ	
	P0 = 0x00;		//����P0���������
	P1 = 0xfe;		//ѡ�������������ʮλ
	P0 = seg_table[seg_EW/10];
	Delay1ms();
	
	P1 = 0xff;
	P0 = 0x00;
	P1 = 0xfd;		//ѡ������������ܸ�λ
	P0 = seg_table[seg_EW%10];
	Delay1ms();
	
	P1 = 0xff;
	P0 = 0x00;
	P1 = 0xfb;  	//ѡ���ϱ����������ʮλ
	P0 = seg_table[seg_SN/10];
	Delay1ms();
	
		
	P1 = 0xff;
	P0 = 0x00;
	P1 = 0xf7;  	//ѡ���ϱ���������ܸ�λ
	P0 = seg_table[seg_SN%10];
	Delay1ms();
	
}

/*��ʱ��T0�жϺ���*/
void InterruptTimer0(void) interrupt 1
{
	/*�Ƶƿ��Ʋ���*/
	static unsigned char count0 = 0;
	if (seg_EW == seg_SN || (seg_EW == 0 && seg_SN != 0))		//��������Ƶƿ�ʼ��˸
		if (count0 % 10 == 0)				//�Ƶ�һ����˸һ��,0.5s����0.5s��
			yellow_EW = !yellow_EW;
	if (seg_EW == seg_SN || (seg_SN == 0 && seg_EW != 0))		//�ϱ�����Ƶ���˸
		if (count0 % 10 == 0)
			yellow_SN = !yellow_SN;
	

	/*����ܲ���*/
	/*�ƵƵ���ʱ�����л�״̬1,2*/
	if (seg_EW == 0 && seg_SN == 0)		//�ƵƵ���ʱ����
	{
		if (mode_chosen)				//��������ƵƵ���ʱ����
		{
			seg_EW = DEFAULT_SEG_SN;		//��һ������Ĭ�϶��������ȻƵƣ�
			seg_SN = DEFAULT_SEG_EW;		//�ʽ������������ϱ�����Ĭ��ʱ�以�����л��ƵƷ���
			mode_chosen = !mode_chosen;					//�Ƶ��������Ϊ�ϱ�
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
	
	/*�Ƶƿ�ʼ����ʱ*/
	if (seg_EW == 0 && seg_SN !=0)		//��������ƵƵ���ʱ�����ϱ�������̵Ƶ���ʱ
	{
		seg_EW = seg_SN;
		update();
	}
	
	if (seg_SN == 0 && seg_EW !=0)		//�ϱ�����ƵƵ���ʱ���ڶ���������̵Ƶ���ʱ
	{
		seg_SN = seg_EW;
		update();
	}	
	

	if (count0 == 20)	//һ��ʱ
	{
		count0 = 0;						//���count0
		seg_EW -= 1;
		seg_SN -= 1;
		update();
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
		}
	}

	if (k6 == 0)	//ȷ�ϸ��Ĳ�����
	{
		Delay10ms();
		if (k6 == 0)
		{
			if (seg_EW > seg_SN)
				mode_chosen = 1;
			else
				mode_chosen = 0;
			TR0 = 1;	//������ʱ��0����ʱ�俪ʼ����
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



