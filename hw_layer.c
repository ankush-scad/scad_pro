#include "main.h"
#include "extern.h"
//const unsigned long subval_tbl_8dig[8] = {10000000,1000000,100000,10000,1000,100,10,1};
const unsigned int subval_tbl[4] = {1000,100,10,1};
const unsigned int subval_chartbl[3] = {100,10,1};
const unsigned char onebit_high_tbl2[8] =  {0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01};

const unsigned char onebit_high_tbl[8] =  {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};
//const unsigned char onebit_low_tbl[8] =  {0xfe,0xfd,0xfb,0xf7,0xef,0xdf,0xbf,0x7f};
const unsigned char bcd_ascii_tbl[16] = {"0123456789ABCDEF"};


//----------------------------------------------------------------------------//

void update_595(void)
{    
    unsigned char i,j,tmp,tmp2;
    unsigned char led_buffer_595[6] ;//= {0xaa,0xa0,0x0b,0xab,0x55,0xff};	
//    unsigned char relay_buffer_595[3] ;//= {0x40,0x08,0xcb};	23-07-19
    for(j=0; j<6; j++)
    {	
	//23-7-19
//	if(j<3)
//	{
//	    relay_buffer_595[j] = logic_var.repeat_rly[2 - j].bytes;// 1 = led_on
//	    tmp2 = relay_buffer_595[j];
//	}	
	
	led_buffer_595[j] = logic_var.led_byte[5-j].bytes;// 1 = led_on
	tmp = led_buffer_595[j];
	
	CLK_595_LEDS = 0;
//	CLK_595_RELAY = 0;      //23-07-19
	asm("nop");
	asm("nop");        
	
	for(i=0; i<8; i++)
	{
	    
//	    if(j<3)
//	    {
//		if((tmp2 & onebit_high_tbl[i])== onebit_high_tbl[i]) 
//		    DATA_595_RELAY = 1;
//		else
//		    DATA_595_RELAY = 0;
//
////		CLK_595_RELAY = 1;      //23-7-19
//		asm("nop");
//		asm("nop");
//	
////		CLK_595_RELAY = 0;      //23-7-19
//		asm("nop");
//		asm("nop");
//	    }	    
	    
	    if((tmp & onebit_high_tbl2[i])== onebit_high_tbl2[i]) 
		DATA_595_LEDS = 1;
	    else
		DATA_595_LEDS = 0;

	    
	    CLK_595_LEDS = 1;
	    asm("nop");
	    asm("nop");
	
	    CLK_595_LEDS = 0;
	    asm("nop");
	    asm("nop");	
	}
    }
    
//    STB_595_RELAY = 1;      //23-7-19
    STB_595_LEDS = 1;
    asm("nop");
    asm("nop");
        
//    STB_595_RELAY = 0;      //23-7-19
    STB_595_LEDS = 0;
    asm("nop");
    asm("nop");			
           
}

//----------------------------------------------------------------------------//

void update_repeat_relay(void)
{
//    unsigned char rly_buffer_595[3];
    unsigned char i,j,tmp; 
    unsigned char relay_buffer_595[3];
    for(j = 0; j < 3; j++)
    {
//        tmp = logic_var.repeat_rly[2-j].bytes;  // 0  = relay_on 
        relay_buffer_595[j] = logic_var.repeat_rly[2 - j].bytes;// 1 = led_on
	    tmp = relay_buffer_595[j];
        
        CLK_595_RELAY = 0;
        asm("nop");
        asm("nop");        

        for(i=0; i<8; i++)
        {
            if((tmp & 0x80) == 0x80)
//	    if((tmp & 0x01) == 0x01)
                DATA_595_RELAY = 1;
            else
                DATA_595_RELAY = 0;

            CLK_595_RELAY = 1;
            asm("nop");
            asm("nop");
            CLK_595_RELAY = 0;
            asm("nop");
            asm("nop");
            tmp = tmp << 1;
//	    tmp = tmp >> 1;
            asm("NOP");asm("NOP");
        }
    }
    
    STB_595_RELAY = 1;
    asm("nop");
    asm("nop");

    STB_595_RELAY = 0;
    asm("nop");
    asm("nop");     
}

//----------------------------------------------------------------------------//

void read_165_fault(void)
{
    unsigned char i,j,Data = 0;
    
    STB_165_FAULT = 1;
    asm("nop"); asm("nop"); 
    STB_165_FAULT = 0;
    asm("nop"); asm("nop"); asm("nop");
    asm("nop"); asm("nop"); asm("nop");
    STB_165_FAULT = 1; 
    asm("nop"); asm("nop");
    for(j = 0; j < 3; j++)
    {
        Data = 0;
	for(i = 0; i < 8; i++)
	{
	    Data = Data << 1;
	    if(DATA_165_FAULT == 1)
            {
               Data |= 0x01;
               //spare_pin = 1;
            }
            
            
            CLK_165_FAULT = 1;
            asm("nop"); asm("nop"); asm("nop");
            asm("nop"); asm("nop"); asm("nop");
	    CLK_165_FAULT = 0;
        }    
//        logic_var.input_array[j].bytes  = Data;
	logic_var.input_buffer_165[j] = Data;
    }
}

//----------------------------------------------------------------------------//

void read_165_dip(void)
{
    unsigned char i,j,Data = 0;
    
    STB_165_DIP = 1;
    for(i=0;i<50;i++)
    {
        asm("nop"); asm("nop"); asm("nop");
        asm("nop"); asm("nop"); asm("nop");
    }
    STB_165_DIP = 0;
    for(i=0;i<50;i++)
    {
        asm("nop"); asm("nop"); asm("nop");
        asm("nop"); asm("nop"); asm("nop");
    }
    STB_165_DIP = 1; 
    for(i=0;i<50;i++)
    {
        asm("nop"); asm("nop");asm("nop");
        asm("nop"); asm("nop"); asm("nop");
    }
    for(j = 0; j < 6; j++)
    {
        Data = 0;
        for(i = 0; i < 8; i++)
        {
            Data = Data << 1;
            if(DATA_165_DIP == 1)
                {
                   Data |= 0x01;
                   //spare_pin = 1;
                }


                CLK_165_DIP = 1;
                for(i=0;i<20;i++)
                {
                    asm("nop"); asm("nop"); asm("nop");
                    asm("nop"); asm("nop"); asm("nop");
                }
                CLK_165_DIP = 0;
        }    

	logic_var.input_buffer_165[j+3] = Data;
    }
}

//----------------------------------------------------------------------------//

void send_byte(unsigned char send_byte)
{    
    unsigned int i;  
    while((TX2IF==0)){}
    TX2IF = 0;
    TXEN_hbit = 1;
    MAX_485_PIN2 = 1;   //IF THIS PIN IS HIGH ,TX IS ENABLE

    TXREG2 = send_byte;
    for(i=0;i<450;i++)
    {
	asm("nop");
    }        
    MAX_485_PIN2 = 0;
}

//----------------------------------------------------------------------------//

void hex2asc(unsigned char n)
{
    unsigned char tmp;
    
    tmp = n >> 4;
    tmp &= 0x0f;
    bcd_array[0] = bcd_ascii_tbl[tmp];

    tmp = n;
    tmp &= 0x0f;
    bcd_array[1] = bcd_ascii_tbl[tmp];      
}

//----------------------------------------------------------------------------//

void bcd_array_ascii(unsigned char c)
{
    unsigned char i;

    for(i=0; i!=c; i++)
    {
	bcd_array[i] += 0x30;
    }
}

//----------------------------------------------------------------------------//

void clr_bcd_array(void)
{
    unsigned char i;
	
    for(i=0; i<8; i++)
	bcd_array[i] = 0;

}

//----------------------------------------------------------------------------//

void int2bcd_4dig(unsigned int intval)
{
    //hex 2 bcd of 4 dig.
    //stores result in BCD_ARRAY
    //BCD_ARRAY[0] is the MSD
    unsigned char i;

    clr_bcd_array();

    for(i=0; i!=3; i++)
    {
	while(intval >= subval_tbl[i])
	{
	    intval -= subval_tbl[i];
	    bcd_array[i] = bcd_array[i] + 1;
	}
    }
    bcd_array[3] = intval;
}

//----------------------------------------------------------------------------//

void intval_on_pc(unsigned int uitemp)
{
    int2bcd_4dig(uitemp);
    send_byte(bcd_array[0] + 0x30);
    send_byte(bcd_array[1] + 0x30);		
    send_byte(bcd_array[2] + 0x30);		
    send_byte(bcd_array[3] + 0x30);
    send_byte('\n');
    send_byte('\r');    
}

//----------------------------------------------------------------------------//

void char2bcd_3dig(unsigned char charval)
{
    //hex 2 bcd of 4 dig.
    //stores result in BCD_ARRAY
    //BCD_ARRAY[0] is the MSD

    unsigned char i;

    clr_bcd_array();

    for(i=0; i!=2; i++)
    {
	while(charval >= subval_chartbl[i])
	{
	    charval -= subval_chartbl[i];
	    bcd_array[i] = bcd_array[i] + 1;
	}
    }
    bcd_array[2] = charval;
}

//----------------------------------------------------------------------------//

void char_val_pc(unsigned char val)
{
    char2bcd_3dig(val);
    send_byte(bcd_array[0] + 0x30);
    send_byte(bcd_array[1] + 0x30);		
    send_byte(bcd_array[2] + 0x30);		
    send_byte('\n');
    send_byte('\r');
}

//----------------------------------------------------------------------------//

void dicing_slicing(void)
{
    unsigned char tmp1,tmp2,tmp3;
    unsigned char m,n;
    unsigned char i,j;

    for(j=0;j<9;j++)
    {
        m = logic_var.input_buffer_165[j];					//binary inputs
        n = 0;
        for(i=0; i<8; i++)
        {	
            if(m & onebit_high_tbl[i])
            {
                n |= onebit_high_tbl[7-i];
            }       
        }
//	logic_var.input_array[j].bytes = logic_var.input_buffer_165[j];					//binary inputs
        logic_var.input_array[j].bytes = n;// tmp1 | tmp2;   //fault
    }
}

//----------------------------------------------------------------------------//

unsigned char calculate_parity(unsigned char val)
{

    unsigned char i,counter;
    counter = 0;

    for(i=0;i<8;i++)
    {
        if((val & 0x01) == 0x01)
            counter++;
	    val >>= 1;
    }

    return(counter & 0x01);  
}

//----------------------------------------------------------------------------//

//void test_fault(void)
//{
//    unsigned char tmp1,tmp2,tmp3;
//    unsigned char m,n;
//    unsigned char i,j;
//                
//    for(j=0;j<6;j++)
//    {
//	if(j == 0)
//	{	    
//	    for(i=0;i<8;i++)
//	    {
//		if(logic_var.input_array[0].bytes & onebit_high_tbl[i])
//		{
//		    logic_var.led_byte[j].bytes &= ~onebit_high_tbl[i];
//		}
//		else		
//		    logic_var.led_byte[j].bytes |= onebit_high_tbl[i];
//	    }
//	}
//	else if(j == 2)
//	{	
//	    for(i=0;i<4;i++)
//	    {
//		if(logic_var.input_array[1].bytes & onebit_high_tbl[i])
//		{
//		    logic_var.led_byte[j].bytes &= ~onebit_high_tbl[i];
//		}
//		else		
//		    logic_var.led_byte[j].bytes |= onebit_high_tbl[i];
//	    }
//	}
////	else if(j == 1)
////	{	    
////	    for(i=0;i<8;i++)
////	    {
////		if(logic_var.input_array[0].bytes & onebit_high_tbl2[i])
////		{
////		    logic_var.led_byte[j].bytes &= ~onebit_high_tbl[i];
////		}
////		else		
////		    logic_var.led_byte[j].bytes |= onebit_high_tbl[i];
////	    }
////	}
////	else if(j == 3)
////	{	
////	    for(i=0;i<4;i++)
////	    {
////		if(logic_var.input_array[1].bytes & onebit_high_tbl2[i])
////		{
////		    logic_var.led_byte[j].bytes &= ~onebit_high_tbl[i];
////		}
////		else		
////		    logic_var.led_byte[j].bytes |= onebit_high_tbl[i];
////	    }
////	}	
//    }
//}



//void long2bcd_8dig(unsigned long longval)
//{
//    unsigned char i;
//
//    clr_bcd_array();
//
//    for(i=0; i<7; i++)
//    {
//	while(longval >= subval_tbl_8dig[i])
//	{
//	    longval -= subval_tbl_8dig[i];
//	    bcd_array[i] = bcd_array[i] + 1;
//	}
//    }
//    bcd_array[7] = longval;
//}
//
//void int_long_send2pc(unsigned long long_val,char num_count)
//{
//    unsigned char i = 0;
//
//    long2bcd_8dig(long_val);
//    for(i = 0;i < num_count ;i++)
//    {
//	send_to_pc(bcd_array[8 - num_count + i] + 0x30);
//    }
//    send_to_pc(' ');
//
//}
//
//void long_val_pc(unsigned long ultemp)
//{
//	
//    long2bcd_8dig(ultemp);	
//    send_to_pc(bcd_array[0] + 0x30);
//    send_to_pc(bcd_array[1] + 0x30);		
//    send_to_pc(bcd_array[2] + 0x30);		
//    send_to_pc(bcd_array[3] + 0x30);
//    send_to_pc(bcd_array[4] + 0x30);
//    send_to_pc(bcd_array[5] + 0x30);		
//    send_to_pc(bcd_array[6] + 0x30);		
//    send_to_pc(bcd_array[7] + 0x30);
//    send_to_pc('\n');
//    send_to_pc('\r');
//}

//----------------------------------------------------------------------------//
//const unsigned char danceled_tbl[4] = {0x02,0x08,0x10,0x40};
//void dance_leds(void)
//{
//    unsigned char i,j;    
//    
//    i = j = 0;
//    while(1)
//    {
//	basems_wait();
//	for(i=0; i<8; i++)
//	{
//	    logic_var.led_byte[j].bytes = onebit_high_tbl[i];
//	    delay_ms(1000);	    
//	}
//	logic_var.led_byte[j].bytes = 0;
//	j++;
//	if(j > 3)
//	    j = 0;
//   
//    }
//}
//
////------------------------------------------------------------------------------

