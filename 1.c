#include <reg52.h>
#define uchar unsigned char
#define uint unsigned int
uchar code led_modA[11] = {0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0x40};	// 0 1 2 3 4 5 6 7 8 9 -
uchar code led_modB[11]={0xbf,0x86,0xdb,0xcf,0xe6,0xed,0xfd,0x87,0xff,0xef,0x00}; //��С����Ĺ�������ܶ���
uchar code wela[8] = {0xfe,0xfd,0xfb,0xf7,0xef,0xdf,0xbf,0x7f};		   ////0����λѡ
uchar time[8] = {0,0,10,0,0,10,0,0};
//1111 1110 

sbit EN =  P2^0;
sbit RW =  P2^1;
sbit RS =  P2^2;
sbit TP =  P2^3;
/*
	�����ⲿ�ж�0.
*/

sbit beep = P2^7;
//ȫ��DQ����TP.

/*
	�����ⲿ�ж�1���������й�.
*/
uint second = 0;	   //��������
uint alarmsecond = 5;	//��������
uchar count = 0;	 	//����

bit mode = 0;			//�л�ģʽ
uint setAlarm = 0;		//��������Ĭ�ϣ�0Ĭ�ϲ�����.1�������ӣ�2ȷ��.

unsigned char tempL=0; 	//��ȫ�ֱ������¶ȸ�8λ
unsigned char tempH=0; 
unsigned int Sdata;		//���������¶ȵ���������
unsigned char xiaoshu; //С����һλ

uchar code DisplayNums[] = "0123456789-";
uchar code DisplayTips[] = "Temp";
uchar code DisplayAlarm[] = "Alarm:";
uchar code DisplayAlarmOpenStatus[] = "open ";
uchar code DisplayAlarmCloseStatus[] = "close";


void delay(unsigned char i);	 			//�ӳ�΢��
void delay1ms(int Z);						//��ʱ����
void onTimeAlarm();							//�������㱨ʱ
void alarmClock();							//��������
void showTime(uint temporaryTime);			//��ʾʱ��.
ReadOneChar(void);							//��һ���ֽ�
void WriteOneChar(unsigned char dat) ;		//дһ���ֽ�
void Init_DS18B20(void) ;					//��ʼ���¶ȴ�����
void ReadTemperature(void) ;				//���¶�ֵ
void showTemperature(unsigned int date);	//��ʾ�¶�ֵ
void Write_Com(uchar Com);					//Һ��дָ��
void Write_Data(uchar Data);				//Һ��д����
void Lcd_Init();							//Һ����ʼ��
void Display_Temperature(uint Data);		//Һ����ʾ�¶�
void Display_Static_Temperature();			//Һ����ʾ��̬����  
void main(){
	TMOD = 	0x21; 	//
	TH0 = (65536-50000)/256;
	TL0 = (65536-50000)%256;
	TH1 = 0xfd;//���ó�ֵ 9600=(2 �� SMOD �η�/32)*��TI ����ʣ�=��1/32��*11059200/(256-X)*12
	TH0 = 0xfd;
	SM0 = 0;
	SM1 = 1;
	REN = 1;   //�����ڽ���
	EA = 1;		//���ж�
	EX0 = 1;   	//�ⲿ���ж�
	IT0 = 1;	//�½���

	ET0 = 1;	//��ʱ���ж�
	TR0 = 1;
	TR1 = 1;
	ES = 1;	//���������ж�
	//delay1ms(750);	//����˵��������������ʱ750ms������85��
	Lcd_Init();
	Display_Static_Temperature();	  //�¶Ȼ�����.ע�Ͷ�?
	while(1){
		ReadTemperature(); //��ȡ�¶�
		if (second%3600==0)	
			onTimeAlarm();	//���
		else if (alarmsecond==0 && setAlarm==2)	  	//��ȷ�Ͻ�������
			alarmClock();	//���,char������==1,==2����
		if (mode==0){
			if (setAlarm==1){
			   showTime(alarmsecond);	
			}
			else if (second%10>=8 && second%10<=9){
				showTemperature(Sdata);		  //��ʾ�¶� 
			}
			else{
				showTime(second);	  //��ʾ�Լ���ʱ��
			}	
		}else{
		  	Display_Temperature(Sdata);	 //Һ����ʾ������ܶ�̬��ʾ��ͻ������ֻ�ܶ�ѡһ
		}
	}
}
void onTimeAlarm(){
	uint i=0,j=0,k=0;
	//���㱨ʱ...
	k = second/3600;
	while(k--){
		for(i=0;i<2;i++){
		  	beep=~beep;	//��һ��.
			for(j=0;j<20;j++)	showTime(second); 	//�޸�ʱ��Ϊ����
		}
	}
	beep=1;
}
void alarmClock(){
	//ʱ�䵽.�ظ���5��.
	uint i=0,j=0;
	uint mycount = 5;
	while(mycount--){
		for(i=0;i<2;i++){
			beep=~beep;	//��һ��.
			for(j=0;j<20;j++)	showTime(alarmsecond);	//�޸�ʱ��Ϊ����,Һ����ʾִ��ʱ������,,,
		}	
	}
	beep=1;
	setAlarm = 0;	//���ӵ��˻����ã�
	alarmsecond = 5;	//�ָ�һ��ʼ��״̬.
}
void showTemperature(unsigned int date)
{ 
	/*
		����ܾã�֮ǰ�����⣬����û��.
	*/
	//P1=0xfb;     //P1.0=0��ѡͨ��һλ
	//P0=seg7code[date/10];  //ʮλ����������
	P1 = wela[5];
	P0 = led_modA[date/10];
	delay1ms(2);
	P1=0xff;
	P1 = wela[6];
	P0 = led_modB[date%10];
	delay1ms(2);
	P1=0xff;
	P1 = wela[7];
	P0 = led_modA[xiaoshu];
	delay1ms(2);
	P1=0xff;
}
/*
	�ӳ�1ms
*/
void delay1ms(int Z){												                                                                               
	int i=0;
	while(Z--){
		for(i=0;i<113;i++);
	}
}
/*
	�ӳ�1us
*/
void delay(unsigned char i)
{
	 for(i;i>0;i--);
}
void showTime(uint temporaryTime){
	/*
		��������,����370s,����370%60 = 10.����370/60,Сʱ370%3600.
		�����������Ӻ󣬻�û������ֻ��ʾ��Ҫ��ʾ�ģ������Ҳ���.
	*/
	char k=0;
	if (temporaryTime<=0)	temporaryTime = 0;	//���������������Լ�����ֹ���ֱ��.
	time[0] = (temporaryTime/3600)/10;	  //���18Сʱ?
	time[1] = (temporaryTime/3600)%10;
	time[3] = (temporaryTime%3600)/60/10;	//�ֳ���60����������59�� = 59x60 = 	3540s
	time[4] = (temporaryTime%3600)/60%10;
	time[6] = (temporaryTime%60)/10;
	time[7] = (temporaryTime%60)%10;
	if (mode==1){
		//��ʼд.
		Write_Com(0x80+0x40+8);
		for(k=0;k<8;k++)
			Write_Data(DisplayNums[time[k]]);
	}else{
		for(k=0;k<8;k++){
			P1=wela[k];
			P0=led_modA[time[k]];
			delay1ms(2);
		}
		P1 = 0xff;
	}
		  
}
/*
	������ʱ��0�жϣ�����ÿ��1s��������1.
	ÿ�ζ�ʱ50ms��1s�о���20���ж�.
*/
void changeTime() interrupt 0{
	/*
		����Ҫȷ�ϼ�����ԭ���������Ӽ��ɣ������ܿ����˳��ж�.
		ȡ��4λ�����,������Ԥ��0111 0000 = 0x70
		add_hour 	x110   0x60
		add_minute 	x101   0x50
		add_second  x100   0x40
		change_mode x011   0x30
		setclock    x010   0x20
		confirm     x001   0x10
	*/
	uchar status = P2&0x70;
	uint temporarysecond = 0;
	//һ��ʼsetAlarm=0,Ĭ�ϼ�����,������setAlarmʱ,alarmsecond = second,������ʾ��������.
	switch(status){
		case 0x60:	temporarysecond = temporarysecond + 3600;break;
		case 0x50:  temporarysecond = temporarysecond + 60;break;
		case 0x40:  temporarysecond = temporarysecond + 1;break;
		case 0x30:  mode = !mode;break;
		case 0x20:	setAlarm = 1;break;	 //�����ǿ�ʼ��������
		case 0x10:	setAlarm = 2;break;  //������ȷ������.
		case 0x00:	break;
	}
	//һ����ʱ����,Ȼ���ټ��ϳ�ֵ����.
	if (setAlarm==1){
		//��������.
		alarmsecond = alarmsecond + temporarysecond;
	}else{
		second = second + temporarysecond;
	}
}
void timer0() interrupt 1{
	TH0 = (65536-50000)/256;
	TL0 = (65536-50000)%256;
	if (count++==20){
		count = 0;
		second = second + 1;	//�ж�hour = 24Сʱ.
		if (setAlarm==2){	   //����ִ��?
			//��ʼ��ʱ.
			alarmsecond = alarmsecond - 1;
		}
	}
}
void setAlarmBySerialPort() interrupt 4{
	/*
		���մ���,��������,��ʽ��00-00-00,�����ն�.
		��ʽ1��10λ�첽�շ�����ʱ��Ҫ���ö�ʱ��1
	*/
	char k=0;
	for(k=0;k<8;k++){
		while(!RI);	//�Ƚ����������.
		RI=0;	  //���յ����ݣ������0
		time[k] = SBUF;
	}

}
void Init_DS18B20(void) 
{
	unsigned char x=0;
	TP=1; //DQ���ø� 
	delay(8); //����ʱ
	TP=0; //���͸�λ���� 
	delay(80); //��ʱ��>480us) 
	TP=1; //���������� 
	delay(5); //�ȴ���15~60us) 
	x=TP; //��X��ֵ���жϳ�ʼ����û�гɹ���18B20���ڵĻ�X=0������X=1 
	delay(20); 
} 
ReadOneChar(void)  //�����������ȴӸ������͵�ƽ1us���ϣ���ʹ��������Ϊ�ߵ�ƽ���Ӷ��������ź�
{
	unsigned char i=0; //ÿ����������̵ĳ���ʱ��Ϊ60us������������֮�������1us���ϵĸߵ�ƽ�ָ���
	unsigned char dat=0; 
	for (i=8;i>0;i--) //һ���ֽ���8λ 
	{
		TP=1; 
		delay(1); 
		TP=0;
		dat>>=1; 	   //������λ
		TP=1; 
		if(TP) 
		dat|=0x80; 	   //�ö�ȡ�����ߵõ�һ��״̬
		delay(4);
} 
return(dat);
} 

//*********************** **дһ���ֽ�**************************//

void WriteOneChar(unsigned char dat) 
{ 
	unsigned char i=0; //�����ߴӸߵ�ƽ�����͵�ƽ������д��ʼ�źš�15us֮�ڽ�����д��λ�͵��������ϣ�
	for(i=8;i>0;i--) //��15~60us֮��������߽��в���������Ǹߵ�ƽ��д1����д0������ 
	{
		TP=0; //�ڿ�ʼ��һ��д����ǰ������1us���ϵĸߵ�ƽ�ָ��ڡ� 
		TP=dat&0x01;  //����λ��������
		delay(5); 
		TP=1; 
		dat>>=1;
	} 
	delay(4);
} 
void ReadTemperature(void) 
{ 
	/*
		��ȷһ�㣬������¶���Ҫ��ʱ��
		1. delay(103)
		2. delay(44)
		3. delay(44)
		4. delay(125)
		5. delay(103+44+44)
		6. delay(80)
	*/
	Init_DS18B20(); //��ʼ��
	WriteOneChar(0xcc); //���������кŵĲ���
	WriteOneChar(0x44); //�����¶�ת��
	delay(125); //ת����Ҫһ��ʱ�䣬��ʱ
	 
	Init_DS18B20(); //��ʼ��
	WriteOneChar(0xcc); //���������кŵĲ��� 
	WriteOneChar(0xbe); //���¶ȼĴ�����ͷ����ֵ�ֱ�Ϊ�¶ȵĵ�λ�͸�λ�� 
	tempL=ReadOneChar(); //�����¶ȵĵ�λLSB
	tempH=ReadOneChar(); //�����¶ȵĸ�λMSB 

	Sdata = tempL/16+tempH*16;      //��������
	xiaoshu = (tempL&0x0f)*10/16; //С����һλ
}


/**********LCD����******************/
void Write_Com(uchar Com){
	/*
		Һ��дָ���Ӧ����
	*/
	RS = 0;
	P0 = Com;
	delay1ms(5);
	EN = 1;
	delay1ms(5);
	EN = 0;
	delay1ms(5);
}
void Write_Data(uchar Data){
	/*
		Һ��д����
	*/
	RS = 1;
	P0 = Data;
	delay1ms(5);
	EN = 1;
	delay1ms(5);
	EN = 0;
	delay1ms(5);
}
void Lcd_Init(){
	EN = 0;
	RW = 0;
	Write_Com(0x38);	//���� 16*2 ��ʾ��ÿ���ַ���5*7 ����8 λ���ݽӿ�
	Write_Com(0x0c);    //����ʾ������ʾ���
	Write_Com(0x06);    //��ַ�� 1����д�����ݵ�ʱ��������
	Write_Com(0x01);    //����,���֮ǰ������.
}
void Display_Temperature(uint Data){
	//Dataת������λ��
	//Data = 0x;
	uint Decade,Uint,k;
	Write_Com(0x80+11);
	for(k=0;k<5;k++){
		if (setAlarm==2){
			Write_Data(DisplayAlarmOpenStatus[k]);	
		}else{
			Write_Data(DisplayAlarmCloseStatus[k]);	
		}	
	}
	Decade = Data/10;
	Uint = Data%10;
	/*	0x80�ǵ�һ�еĵ�һ���ַ���0x80+1�ǵ�һ�еڶ���,0x80+0x40�ǵڶ��е�һ����ַ
		д����ʾ��ַʱ��Ҫ�����λD7��Ϊ�ߵ�ƽ,��1000 0000 + 0100 0000 = 0x80 + 0x40
		��Ƭ���涨
	*/
	//Write_Com(0x80+0x40+5);
	//��Ҫ�Ľ���̫������.
	Write_Com(0x80+0x40);	
	Write_Data(DisplayNums[Decade]);	//д������.
	Write_Data(DisplayNums[Uint]);
	Write_Data('.');
	Write_Data(DisplayNums[xiaoshu]);
	Write_Data('^');	//��C
	Write_Data('C');
	showTime(second);
	//�������������õ�������5�£���ʱ�䲻�ԣ���ÿ5s��һ��.
}

void Display_Static_Temperature(){
	uchar i;
	Write_Com(0x80);
	for(i=0;i<4;i++){
		Write_Data(DisplayTips[i]);
	}
   	Write_Com(0x80+5);
	for(i=0;i<6;i++){
		Write_Data(DisplayAlarm[i]);
	}
}
