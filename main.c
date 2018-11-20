/*
 * svm_v7.c
 *
 * Created: 6/28/2018 2:48:45 AM
 * Author : Rony
 */ 


#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
//defines
#define F_CPU           8000000UL
#define F_timer         1000000ul
#define  F_ref           5
#define Mod_Index       .1
#define  sqrt3          1.7320
#define  sqrt2          1.4142
#define  PI             3.1415
#define point           48

// look up variable

volatile unsigned char n_sample=0,flag =0,cycle=0,j=0,start_flag=1;
volatile uint16_t pre_calculated_duty_a=0,pre_calculated_duty_b=0,pre_calculated_duty_c=0; 
volatile float sin_forward[]= {0,0.1305,0.2588,0.3827,0.5,0.6088,0.707,0.7934,0,0.1305,0.2588,0.3827,0.5,0.6088,0.707,0.7934,0,0.1305,0.2588,0.3827,0.5,0.6088,0.707,0.7934,0,0.1305,0.2588,0.3827,0.5,0.6088,0.707,0.7934,0,0.1305,0.2588,0.3827,0.5,0.6088,0.707,0.7934,0,0.1305,0.2588,0.3827,0.5,0.6088,0.707,0.7934};
volatile float sin_reverse[]= {0.866,0.7934,0.707,0.6088,0.5,0.3827,0.2588,0.1305,0.866,0.7934,0.707,0.6088,0.5,0.3827,0.2588,0.1305,0.866,0.7934,0.707,0.6088,0.5,0.3827,0.2588,0.1305,0.866,0.7934,0.707,0.6088,0.5,0.3827,0.2588,0.1305,0.866,0.7934,0.707,0.6088,0.5,0.3827,0.2588,0.1305,0.866,0.7934,0.707,0.6088,0.5,0.3827,0.2588,0.1305};



//function prototype

void set_up_timer(int ICR_value);


int main(void)
{
	DDRB |= (1<<PINB0)|(1<<PINB5)|(1<<PINB6)|(1<<PINB7);  
	DDRC |=(1<<PINC0)  ;        // OC1A & OC1B,OC1C - PWM1 & PWM2 &PWM3
	PORTB |=(1<<PINB0);
	
	int sampling_frequency= F_ref*point;
	int ICR_value=(int)(0.5*(F_timer/(sampling_frequency)));//multiply by 0.5 cause phase correct pwm,top value =0.5*total sample time
	float a_Ts=ICR_value*Mod_Index ;
	uint8_t i=0;
	for (i=0;i<=47;i++)
	{
		sin_forward[i]=sin_forward[i]*a_Ts;
		sin_reverse[i]=sin_reverse[i]*a_Ts;
		
	}
	PORTB &=~(1<<PINB0);
	set_up_timer(ICR_value);
	
	/*  application code */
	while (1)
	{
		
	}
}
void set_up_timer(int ICR_value)
{
	TCCR1A |=(1<<COM1A1)|(1<<COM1A0)|(1<<COM1B1)|(1<<COM1B0)|(1<<COM1C1)|(1<<COM1C0)|(1<<WGM11);//mode 10 phase correct pwm,3 output oc1a,oc1b,oc1c
	TCCR1B |=(1<<WGM13)|(1<<CS11);//clk/8
	//for Timer 3
	
	TCCR3A |=(1<<COM3A1)|(1<<COM3A0)|(1<<COM3B1)|(1<<COM3B0)|(1<<COM3C1)|(1<<COM3C0)|(1<<WGM31);
	TCCR3B |=(1<<WGM33)|(1<<CS31);
	
	TIMSK |=(1<<TOIE1);
	
	ICR1 =ICR_value;
	OCR1A=ICR1-50; //just initalizing with something
	OCR1B=ICR1-100;
	OCR1C=ICR1-150;
	uint16_t Tr = (int)(sin_reverse[n_sample]);//these are for sample zero
	uint16_t Tl = (int)(sin_forward[n_sample]);
	uint16_t T0 = ICR1-Tr-Tl;
	uint16_t Tr_Plus_Tl=Tr+Tl;
	uint16_t half_T0= (T0>>1);
	
	
	pre_calculated_duty_a=ICR1-(Tr_Plus_Tl +half_T0);// to be used for sample number zero
	pre_calculated_duty_b=ICR1-(Tl+half_T0);
	pre_calculated_duty_c=ICR1-(half_T0);
	sei();
	
}

ISR(TIMER1_OVF_vect)
{
		PORTB |=(1<<PINB0);
		// assigning duty cycle which was calculated in previous sample
		OCR1A=pre_calculated_duty_a;
		OCR1B=pre_calculated_duty_b;
		OCR1C=pre_calculated_duty_c;
		
		
		
		// duty cycle calculation for next sample
		n_sample++;
	
		if(n_sample==48)
		{
			n_sample=0;
			cycle++;
		}
	
		/* for generating sine wave of (f=10,m=0.5,icr=1042,ats=208),(f=20,m=0.4,icr=521,ats=208)
		,(f=30,m=0.2,icr=347,ats=69),(f=40,m=0.7,icr=260,ats=182),(f=50,m=0.9,icr=208,ats=187) after 2,4,7,11,16 cycles respectively
		Their sine table value coeeficients are 2.54,0.4,0.33,2.62,1.03 respectively

		*/
		//ICR value modification, will take effect after sine value modification in previous cycle.At the beginning of a new cycle
		// when n_sample is zero assinging new ICR value with  modified sine table value of previous cycle
		// will produce desired value for calculating new Tr,Tl etc for new frequency
		
		if (start_flag)
		{
			if (cycle==16)
			{
				ICR1 =208;
				
			}
			else if (cycle==11)
			{
				ICR1 =260;
				
			}
			else if (cycle==7)
			{
				ICR1 =347;
				
			}
			else if (cycle==4)
			{
				ICR1 =521;
				
			}
			else if (cycle==2)
			{
				ICR1 =1042;
				
			}
			
		}
	
	
	// main calculation block start	
	
		uint8_t   sector = (n_sample>>3);  //48 samples is 6 sectors,so 8 samples per sector
		uint16_t Tr = (int)(sin_reverse[n_sample]);
		uint16_t Tl = (int)(sin_forward[n_sample]);
		uint16_t T0 = ICR1-Tr-Tl;
		uint16_t Tr_Plus_Tl=Tr+Tl;
		uint16_t half_T0= 0.5*T0;
	
	
		switch(sector)
		{
			case 0:
			pre_calculated_duty_a=ICR1-(Tr_Plus_Tl +half_T0);
			pre_calculated_duty_b=ICR1-(Tl+half_T0);
			pre_calculated_duty_c=ICR1-(half_T0);
			break;
		
			case 1:
			pre_calculated_duty_a=ICR1-(Tr +half_T0);
			pre_calculated_duty_b=ICR1-(Tr_Plus_Tl+half_T0);
			pre_calculated_duty_c=ICR1-(half_T0);
			break;
		
			case 2:
			pre_calculated_duty_a=ICR1-(half_T0);
			pre_calculated_duty_b=ICR1-(Tr_Plus_Tl+half_T0);
			pre_calculated_duty_c=ICR1-(Tl+half_T0);
			break;
		
			case 3:
			pre_calculated_duty_a=ICR1-(half_T0);
			pre_calculated_duty_b=ICR1-(Tr+half_T0);
			pre_calculated_duty_c=ICR1-(Tr_Plus_Tl+half_T0);
			break;
		
			case 4:
			pre_calculated_duty_a=ICR1-(Tl +half_T0);
			pre_calculated_duty_b=ICR1-(half_T0);
			pre_calculated_duty_c=ICR1-(Tr_Plus_Tl+half_T0);
			break;
		
			case 5:
			pre_calculated_duty_a=ICR1-(Tr_Plus_Tl +half_T0);
			pre_calculated_duty_b=ICR1-(half_T0);
			pre_calculated_duty_c=ICR1-(Tr+half_T0);
			break;
		
			default: break;
		}
		
		// main calculation block end
		
		
		// in previous cycle sine table modification
		
		if(start_flag)
		{
			// sine table modification
			if(cycle==15)
			{
				
				flag=5;
				
				
			}
			else if (cycle==10)
			{
				flag=4;
			}
			else if (cycle==6)
			{
				flag=3;
			}
			else if (cycle==3)
			{
				flag=2;
			}
			else if (cycle==1)
			{
				flag=1;
			}
			else
			{
				flag=0;
			}
			if(flag==5)
			{
				sin_forward[j]=sin_forward[j]*1.03;
				sin_reverse[j]=sin_reverse[j]*1.03;
				j++;
				
			}
			
			else if(flag==4)
			{
				sin_forward[j]=sin_forward[j]*2.62;
				sin_reverse[j]=sin_reverse[j]*2.62;
				j++;
				
			}
			
			
			else if(flag==3)
			{
				sin_forward[j]=sin_forward[j]*0.33;
				sin_reverse[j]=sin_reverse[j]*0.33;
				j++;
				
			}
			
			else if(flag==2)
			{
				sin_forward[j]=sin_forward[j]*0.4;
				sin_reverse[j]=sin_reverse[j]*0.4;
				j++;
				
			}
			
			else if(flag==1)
			{
				sin_forward[j]=sin_forward[j]*2.54;
				sin_reverse[j]=sin_reverse[j]*2.54;
				j++;
				
			}
			
			
			if(j==48)
			{
				flag=0;
				j=0;
			}
			
			
			
			// flag condition changing
			if(cycle>16)
			{
				start_flag=0;
			}
			
		}// if(start_flag) ends here
		
	

		PORTB &=~(1<<PINB0);
	
}

