#include<reg51.h>
#define TRUE  (1)
#define FALSE (0)
#define NUM_KEY  P1
#define GPIO_DIG P0
#define u8 unsigned char
#define u16 unsigned int
u8 press=0;
u8 key_click=0;
u8 send_data=0;
u8 send_next=0;
u8 clock_index =0;
u8 receive_data=0;
u8 receive_done=0;
u8 CLEAR_CONTROL=0xE3;
u8 playerA=0,playerB=0;
u8 SET_POS[8]={0x00,0x04,0x08,0x0C,0x10,0x14,0x18,0x1C};
u8 DIG_CODE[11]={0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x00};
bit releaseMOV=TRUE;
bit releaseATT=TRUE;
sbit  IN=P3^6;
sbit  CL=P3^3;
sbit OUT=P3^7;
sbit ATT=P3^1;
void set(u8 pos)
{
	P2&=CLEAR_CONTROL;
	P2|=SET_POS[8-pos];
}
void idle(u16 us)
{
	u16 x=1;while(x++<=us);
}
void init()
{
	bit A,B;
	EA=0;
	IN=0;CL=0;
	while(TRUE)
	{
		A=CL;idle(10240);B=CL;
		if(A&&B) break;
	}
	EX1=IT1=1;
	EA=1;
}
void flash()
{
	set(1);GPIO_DIG=DIG_CODE[playerA/10];idle(64);GPIO_DIG=0x00;
	set(2);GPIO_DIG=DIG_CODE[playerA%10];idle(64);GPIO_DIG=0x00;

	set(4);GPIO_DIG=0x3E;idle(64);GPIO_DIG=0x00;
	set(5);GPIO_DIG=0x6D;idle(64);GPIO_DIG=0x00;

	set(7);GPIO_DIG=DIG_CODE[playerB/10];idle(64);GPIO_DIG=0x00;
	set(8);GPIO_DIG=DIG_CODE[playerB%10];idle(64);GPIO_DIG=0x00;
}
void event()
{
	bit A,B;
	u8 new_value=0;
	A=ATT,idle(512),B=ATT;
	if(A==B)
	{  	
		if(A&&B) releaseATT=TRUE;
		else if(!releaseATT)
		{
			key_click=0;
			return;
		}
		else if(!A&&!B)
		{
			releaseATT=FALSE,key_click=5;
			return;
		}
	}
	NUM_KEY=0x0F;
	if(NUM_KEY==0x0F)
	{
		key_click=0,releaseMOV=TRUE,press=0;
		return;
	}
	idle(512);
	if(NUM_KEY==0x0F)
	{
		key_click=0,releaseMOV=TRUE,press=0;
		return;
	}
	if(!releaseMOV)
	{
		key_click=0,press+=1;
		if(press>=16) press=0,releaseMOV=TRUE;
		return;
	}
	NUM_KEY=0xF0;
	switch(NUM_KEY)
	{
		case 0x70:new_value= 0;break;
		case 0xB0:new_value= 4;break;
		case 0xD0:new_value= 8;break;
		case 0xE0:new_value=12;break;
	}
	NUM_KEY=0x0F;
	switch(NUM_KEY)
	{
		case 0x07:new_value+=0;break;
		case 0x0B:new_value+=1;break;
		case 0x0D:new_value+=2;break;
		case 0x0E:new_value+=3;break;
	}
	switch(new_value)
	{
		case 10:key_click=1;break;
		case 15:key_click=2;break;
		case 14:key_click=3;break;
		case 13:key_click=4;break;
		default:key_click=6;break;
	}
	releaseMOV=FALSE;		
}
void proc()
{
	if(!send_next) send_next=key_click;
	if(receive_done)
	{
		if(receive_done==1) playerA+=1;
		if(receive_done==2) playerB+=1;
		receive_done=0;
	}
}
void main()
{
	init();while(TRUE)flash(),event(),proc();
}
void clock() interrupt 2
{
	if(!clock_index)
	{
		if(!receive_done) receive_done=receive_data;
		send_data=send_next,send_next=0;
	}
	if(IN)
		receive_data=receive_data| (1<<clock_index);
	else
		receive_data=receive_data&~(1<<clock_index);
	OUT=(send_data>>clock_index)&1;
	clock_index=(clock_index+1)%4;	
}