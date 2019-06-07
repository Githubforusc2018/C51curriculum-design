#include <reg52.h>
#define uchar unsigned char
#define uint unsigned int
uchar code led_modA[11] = {0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0x40};	// 0 1 2 3 4 5 6 7 8 9 -
uchar code led_modB[11]={0xbf,0x86,0xdb,0xcf,0xe6,0xed,0xfd,0x87,0xff,0xef,0x00}; //带小数点的共阴数码管段码
uchar code wela[8] = {0xfe,0xfd,0xfb,0xf7,0xef,0xdf,0xbf,0x7f};		   ////0开启位选
uchar time[8] = {0,0,10,0,0,10,0,0};
//1111 1110 

sbit EN =  P2^0;
sbit RW =  P2^1;
sbit RS =  P2^2;
sbit TP =  P2^3;
/*
	定义外部中断0.
*/

sbit beep = P2^7;
//全部DQ换成TP.

/*
	定义外部中断1，跟闹钟有关.
*/
uint second = 0;	   //定义秒数
uint alarmsecond = 5;	//闹钟秒数
uchar count = 0;	 	//计数

bit mode = 0;			//切换模式
uint setAlarm = 0;		//设置闹钟默认，0默认不设置.1设置闹钟，2确认.

unsigned char tempL=0; 	//设全局变量，温度高8位
unsigned char tempH=0; 
unsigned int Sdata;		//测量到的温度的整数部分
unsigned char xiaoshu; //小数第一位

uchar code DisplayNums[] = "0123456789-";
uchar code DisplayTips[] = "Temp";
uchar code DisplayAlarm[] = "Alarm:";
uchar code DisplayAlarmOpenStatus[] = "open ";
uchar code DisplayAlarmCloseStatus[] = "close";


void delay(unsigned char i);	 			//延迟微妙
void delay1ms(int Z);						//延时毫秒
void onTimeAlarm();							//设置整点报时
void alarmClock();							//设置闹钟
void showTime(uint temporaryTime);			//显示时间.
ReadOneChar(void);							//读一个字节
void WriteOneChar(unsigned char dat) ;		//写一个字节
void Init_DS18B20(void) ;					//初始化温度传感器
void ReadTemperature(void) ;				//读温度值
void showTemperature(unsigned int date);	//显示温度值
void Write_Com(uchar Com);					//液晶写指令
void Write_Data(uchar Data);				//液晶写数据
void Lcd_Init();							//液晶初始化
void Display_Temperature(uint Data);		//液晶显示温度
void Display_Static_Temperature();			//液晶显示静态数据  
void main(){
	TMOD = 	0x21; 	//
	TH0 = (65536-50000)/256;
	TL0 = (65536-50000)%256;
	TH1 = 0xfd;//设置初值 9600=(2 的 SMOD 次方/32)*（TI 溢出率）=（1/32）*11059200/(256-X)*12
	TH0 = 0xfd;
	SM0 = 0;
	SM1 = 1;
	REN = 1;   //允许串口接收
	EA = 1;		//总中断
	EX0 = 1;   	//外部分中断
	IT0 = 1;	//下降沿

	ET0 = 1;	//定时分中断
	TR0 = 1;
	TR1 = 1;
	ES = 1;	//开启串口中断
	//delay1ms(750);	//网上说这是正常现象，延时750ms，消掉85°
	Lcd_Init();
	Display_Static_Temperature();	  //温度会跳变.注释多?
	while(1){
		ReadTemperature(); //读取温度
		if (second%3600==0)	
			onTimeAlarm();	//检测
		else if (alarmsecond==0 && setAlarm==2)	  	//有确认建才有用
			alarmClock();	//检测,char不能用==1,==2这样
		if (mode==0){
			if (setAlarm==1){
			   showTime(alarmsecond);	
			}
			else if (second%10>=8 && second%10<=9){
				showTemperature(Sdata);		  //显示温度 
			}
			else{
				showTime(second);	  //显示自己的时钟
			}	
		}else{
		  	Display_Temperature(Sdata);	 //液晶显示与数码管动态显示冲突，这里只能二选一
		}
	}
}
void onTimeAlarm(){
	uint i=0,j=0,k=0;
	//整点报时...
	k = second/3600;
	while(k--){
		for(i=0;i<2;i++){
		  	beep=~beep;	//响一声.
			for(j=0;j<20;j++)	showTime(second); 	//修改时间为负数
		}
	}
	beep=1;
}
void alarmClock(){
	//时间到.重复响5下.
	uint i=0,j=0;
	uint mycount = 5;
	while(mycount--){
		for(i=0;i<2;i++){
			beep=~beep;	//响一声.
			for(j=0;j<20;j++)	showTime(alarmsecond);	//修改时间为负数,液晶显示执行时间问题,,,
		}	
	}
	beep=1;
	setAlarm = 0;	//闹钟到了还设置？
	alarmsecond = 5;	//恢复一开始的状态.
}
void showTemperature(unsigned int date)
{ 
	/*
		困惑很久，之前有问题，现在没了.
	*/
	//P1=0xfb;     //P1.0=0，选通第一位
	//P0=seg7code[date/10];  //十位数，查表，输出
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
	延迟1ms
*/
void delay1ms(int Z){												                                                                               
	int i=0;
	while(Z--){
		for(i=0;i<113;i++);
	}
}
/*
	延迟1us
*/
void delay(unsigned char i)
{
	 for(i;i>0;i--);
}
void showTime(uint temporaryTime){
	/*
		不用这样,比如370s,秒数370%60 = 10.分数370/60,小时370%3600.
		按下设置闹钟后，或没按，我只显示我要显示的，其他我不管.
	*/
	char k=0;
	if (temporaryTime<=0)	temporaryTime = 0;	//这里是闹钟秒数自减，防止数字变大.
	time[0] = (temporaryTime/3600)/10;	  //最大18小时?
	time[1] = (temporaryTime/3600)%10;
	time[3] = (temporaryTime%3600)/60/10;	//分超过60？假设现在59分 = 59x60 = 	3540s
	time[4] = (temporaryTime%3600)/60%10;
	time[6] = (temporaryTime%60)/10;
	time[7] = (temporaryTime%60)%10;
	if (mode==1){
		//开始写.
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
	开启定时器0中断，定义每个1s，秒数加1.
	每次定时50ms，1s中就是20次中断.
*/
void changeTime() interrupt 0{
	/*
		不需要确认键，在原基础上增加即可，这样能快速退出中断.
		取高4位的情况,进行与预算0111 0000 = 0x70
		add_hour 	x110   0x60
		add_minute 	x101   0x50
		add_second  x100   0x40
		change_mode x011   0x30
		setclock    x010   0x20
		confirm     x001   0x10
	*/
	uchar status = P2&0x70;
	uint temporarysecond = 0;
	//一开始setAlarm=0,默认加秒数,当按下setAlarm时,alarmsecond = second,出现显示出错问题.
	switch(status){
		case 0x60:	temporarysecond = temporarysecond + 3600;break;
		case 0x50:  temporarysecond = temporarysecond + 60;break;
		case 0x40:  temporarysecond = temporarysecond + 1;break;
		case 0x30:  mode = !mode;break;
		case 0x20:	setAlarm = 1;break;	 //这里是开始设置闹钟
		case 0x10:	setAlarm = 2;break;  //这里是确认闹钟.
		case 0x00:	break;
	}
	//一个临时变量,然后再加上初值即可.
	if (setAlarm==1){
		//设置闹钟.
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
		second = second + 1;	//判断hour = 24小时.
		if (setAlarm==2){	   //不会执行?
			//开始计时.
			alarmsecond = alarmsecond - 1;
		}
	}
}
void setAlarmBySerialPort() interrupt 4{
	/*
		接收串口,设置闹钟,格式是00-00-00,虚拟终端.
		方式1，10位异步收发，这时需要设置定时器1
	*/
	char k=0;
	for(k=0;k<8;k++){
		while(!RI);	//等接收数据完毕.
		RI=0;	  //接收到数据，软件清0
		time[k] = SBUF;
	}

}
void Init_DS18B20(void) 
{
	unsigned char x=0;
	TP=1; //DQ先置高 
	delay(8); //稍延时
	TP=0; //发送复位脉冲 
	delay(80); //延时（>480us) 
	TP=1; //拉高数据线 
	delay(5); //等待（15~60us) 
	x=TP; //用X的值来判断初始化有没有成功，18B20存在的话X=0，否则X=1 
	delay(20); 
} 
ReadOneChar(void)  //主机数据线先从高拉至低电平1us以上，再使数据线升为高电平，从而产生读信号
{
	unsigned char i=0; //每个读周期最短的持续时间为60us，各个读周期之间必须有1us以上的高电平恢复期
	unsigned char dat=0; 
	for (i=8;i>0;i--) //一个字节有8位 
	{
		TP=1; 
		delay(1); 
		TP=0;
		dat>>=1; 	   //数据移位
		TP=1; 
		if(TP) 
		dat|=0x80; 	   //得读取数据线得到一个状态
		delay(4);
} 
return(dat);
} 

//*********************** **写一个字节**************************//

void WriteOneChar(unsigned char dat) 
{ 
	unsigned char i=0; //数据线从高电平拉至低电平，产生写起始信号。15us之内将所需写的位送到数据线上，
	for(i=8;i>0;i--) //在15~60us之间对数据线进行采样，如果是高电平就写1，低写0发生。 
	{
		TP=0; //在开始另一个写周期前必须有1us以上的高电平恢复期。 
		TP=dat&0x01;  //按低位发送数据
		delay(5); 
		TP=1; 
		dat>>=1;
	} 
	delay(4);
} 
void ReadTemperature(void) 
{ 
	/*
		精确一点，计算读温度需要的时间
		1. delay(103)
		2. delay(44)
		3. delay(44)
		4. delay(125)
		5. delay(103+44+44)
		6. delay(80)
	*/
	Init_DS18B20(); //初始化
	WriteOneChar(0xcc); //跳过读序列号的操作
	WriteOneChar(0x44); //启动温度转换
	delay(125); //转换需要一点时间，延时
	 
	Init_DS18B20(); //初始化
	WriteOneChar(0xcc); //跳过读序列号的操作 
	WriteOneChar(0xbe); //读温度寄存器（头两个值分别为温度的低位和高位） 
	tempL=ReadOneChar(); //读出温度的低位LSB
	tempH=ReadOneChar(); //读出温度的高位MSB 

	Sdata = tempL/16+tempH*16;      //整数部分
	xiaoshu = (tempL&0x0f)*10/16; //小数第一位
}


/**********LCD部分******************/
void Write_Com(uchar Com){
	/*
		液晶写指令，对应操作
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
		液晶写数据
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
	Write_Com(0x38);	//设置 16*2 显示，每个字符是5*7 点阵，8 位数据接口
	Write_Com(0x0c);    //开显示，不显示光标
	Write_Com(0x06);    //地址加 1，当写入数据的时候光标右移
	Write_Com(0x01);    //清屏,清楚之前的内容.
}
void Display_Temperature(uint Data){
	//Data转换成三位数
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
	/*	0x80是第一行的第一个字符，0x80+1是第一行第二个,0x80+0x40是第二行第一个地址
		写入显示地址时，要求最高位D7恒为高电平,即1000 0000 + 0100 0000 = 0x80 + 0x40
		单片机规定
	*/
	//Write_Com(0x80+0x40+5);
	//需要改进，太复杂了.
	Write_Com(0x80+0x40);	
	Write_Data(DisplayNums[Decade]);	//写入数据.
	Write_Data(DisplayNums[Uint]);
	Write_Data('.');
	Write_Data(DisplayNums[xiaoshu]);
	Write_Data('^');	//°C
	Write_Data('C');
	showTime(second);
	//问题是这样设置的闹钟响5下，但时间不对，即每5s响一下.
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
