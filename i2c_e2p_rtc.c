//------------------------------------------------------------------------------
//------------------------ I2C CODES -------------------------------------------
//------------------------------------------------------------------------------
#include "main.h"
#include "extern.h"

#define _nop_() asm("nop")
const unsigned char rtc_addr[6] = {0x04,0x05,0x06,0x02,0x01,0x00};// date,mon ,year,hrs, min, sec
//----------------------------------------------------------------------------//

void i2c_delay(void)
{
	asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
	asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
	asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
	asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
	asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
	asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
}

//----------------------------------------------------------------------------//

void i2c_shift_out(unsigned char dat, unsigned char cnts)
{
//assumed both sda & scl are held LOW upon entry.
//leaves with both sda & scl high.
//SHIFT OUTS MSBit first.
//NOPs added for suitability with rtc speed
    unsigned char i;

    scl_hbit = LOWBYTE;
    i2c_delay();

    for(i=0; i<cnts; i++)
    {

	if((dat & 0x80) != 0)sda_hbit = 1; else sda_hbit = 0;
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	scl_hbit = HIGHBYTE;
	i2c_delay();
	scl_hbit = LOWBYTE;
	i2c_delay();
	dat <<= 1;
    }

//Extra clock pulse for ACK (9th clk pulse during shift out).
//Its reqd. to chk ack and report error, but its to be implemented later.
//Right now only the 9th pulse issued to the device so that it proceeds as 
//per the protocol.


//  sda_hbit = 1;		//raise sda to allow device to acknowledge (by pulling the hbit),
    SDA_TRISIN;			//test bit to chk device functioning or else report error. IMPLEMENT LATER

    _nop_();
    _nop_();
    _nop_();

    scl_hbit = 1;
    i2c_delay();
    scl_hbit = 0;
    i2c_delay();
    sda_hbit = 0;

    SDA_TRISOUT;
}

//----------------------------------------------------------------------------//

unsigned char i2c_shift_in(unsigned char count)		//Tx:Byte to be shifted in, Rx:No of shifts.
{
//assumed both sda & scl are held LOW upon entry.
//leaves with both sda & scl high.
//SHIFTS IN MSBit first.

    unsigned char in_byte = 0;

    //sda_hbit = 1;
    SDA_TRISIN;

    do
    {
	scl_hbit = 1;
	i2c_delay();
		
	in_byte = ((in_byte << 1)| sda_hbit);
	scl_hbit = 0;
	i2c_delay();

	count--;

    }
    while(count!=0);
//  sda_hbit = 0;

    SDA_TRISOUT;    
    return(in_byte);
}

//----------------------------------------------------------------------------//

void i2c_ackout(void)
{
//Ack out used in array read.
    sda_hbit = 0;
    i2c_delay();
    scl_hbit = 1;
    i2c_delay();
    scl_hbit = 0;
    sda_hbit = 0;
}

//----------------------------------------------------------------------------//

void i2c_no_ackout(void)
{
//No-Ack out used in byte-read, array-read.
    sda_hbit = 1;
    i2c_delay();
    scl_hbit = 1;
    i2c_delay();
    scl_hbit = 0;
    sda_hbit = 0;
}

//----------------------------------------------------------------------------//

void i2c_start(void)
{
//ensure both scl & sda are high, after some delay lower sda while scl is still 1,
//after that some delay and sda also made low.
//leaves with both scl & sda low.
//more NOPs may be reqd. while actual testing depending upon the speed of the device.
//>>>>> Other than start/stop conditions, sda may change only when scl is low <<<<<

    sda_hbit = 1;
    scl_hbit = 1;
    i2c_delay();
    sda_hbit = 0;		//lower sda while scl is 1
    i2c_delay();
    scl_hbit = 0;		//after some delay, lower scl.
}

//----------------------------------------------------------------------------//

void i2c_stop(void)
{
//assumed scl is low upon entry.
//leaves with both scl & sda high.
//more NOPs may be reqd. while actual testing depending upon the speed of the device.

    sda_hbit = 0;		//ensure sda is low.
    i2c_delay();
    scl_hbit = 1;		//raise scl while sda is low
    i2c_delay();
    sda_hbit = 1;		//raise sda while scl is 1
}

//----------------------------------------------------------------------------//

unsigned char i2c_byte_rd(unsigned char dev_addr,unsigned long adr)
{
//Its the duty of the calling code to ensure whether i2c-delay-wr flag is
//enabled or not. Hence it is not checked in this code.
    
    unsigned char temp=0;
    unsigned char addrh,addrl;
    unsigned char control_byte;
    union int_char un1;
    
    if(adr>0xffff)
        control_byte=0XA8;                  //to make 17 bit 1
    else 
        control_byte=0XA0;
    
    adr = (unsigned int)adr;
    un1.int_val = adr;
    addrh = un1.char_val[HIGHBYTE];
    addrl = un1.char_val[LOWBYTE];
    dev_addr=dev_addr<<1;
    control_byte = control_byte | dev_addr;
    
    i2c_start();
    i2c_shift_out(control_byte,8);
    i2c_shift_out(addrh,8);
    i2c_shift_out(addrl,8);
    
    control_byte |= 0x01;
    
    i2c_start();
    i2c_shift_out(control_byte,8);
    temp = i2c_shift_in(8);
    i2c_no_ackout();
    i2c_stop();
    return(temp);
    
}

//----------------------------------------------------------------------------//

unsigned int i2c_int_rd(unsigned char dev_addr,unsigned long adr)
{
    union int_char un1;
    
    un1.char_val[LOWBYTE] = i2c_byte_rd(dev_addr,adr);
    adr++;
    un1.char_val[HIGHBYTE] = i2c_byte_rd(dev_addr,adr);
    return(un1.int_val);
}

//----------------------------------------------------------------------------//

void init_i2c_gpios(void)
{
    SDA_TRIS = 0;
    
    SCL_TRIS = 0;
}

//----------------------------------------------------------------------------//
void i2c_byte_wr(unsigned char dev_addr,unsigned long adr,unsigned char dta)
{
    unsigned char addrh,addrl;
    unsigned char control_byte;
    union int_char un1;
    
    if(adr>0xffff)
        control_byte=0XA8;              //to make 17 bit 1
    else 
        control_byte=0XA0;
    
    adr = (unsigned int)adr;
    un1.int_val = adr;
    addrh = un1.char_val[HIGHBYTE];
    addrl = un1.char_val[LOWBYTE];
    dev_addr=dev_addr<<1;
    control_byte = control_byte | dev_addr;
    
    i2c_start();
    i2c_shift_out(control_byte,8);
    i2c_shift_out(addrh,8);
    i2c_shift_out(addrl,8);
    i2c_shift_out(dta,8);
    i2c_stop();
    
    gen_var.i2c_wr_cntr = 12;
    while(gen_var.i2c_wr_cntr > 0){}       
}

//----------------------------------------------------------------------------//

void i2c_int_wr(unsigned char dev_addr,unsigned long adr,unsigned int dta)
{
    union int_char un1;
    
    un1.int_val = dta;
    i2c_byte_wr(dev_addr,adr,un1.char_val[LOWBYTE]);
    adr++;
    i2c_byte_wr(dev_addr,adr,un1.char_val[HIGHBYTE]);
 }

//----------------------------------------------------------------------------//

void rtc_byte_wr(unsigned char adr,unsigned char dta)
{
    i2c_start();
    i2c_shift_out(0xd0,8);
    i2c_shift_out(adr,8);
    i2c_shift_out(dta,8);
    i2c_stop();
}

//----------------------------------------------------------------------------//
unsigned char rtc_byte_rd(unsigned char adr)
{	
//Its the duty of the calling code to ensure whether i2c-delay-wr flag is
//enabled or not. Hence it is not checked in this code.
	
    unsigned char temp=0;
	
    i2c_start();
    i2c_shift_out(0xd0,8);
    i2c_shift_out(adr,8);
	
    i2c_start();
    i2c_shift_out(0xd1,8);
    temp = i2c_shift_in(8);
    i2c_no_ackout();
    i2c_stop();
    return(temp);
}

//----------------------------------------------------------------------------//

void init_rtc(void)
{
    unsigned char temp,temp2;

    temp = rtc_byte_rd(0);
    temp &= 0x7f;
    rtc_byte_wr(0,temp);		//Disable CLOCK-HALT bit
    rtc_byte_wr(7,0x10);		//force on sqwe

    temp = rtc_byte_rd(63);
    temp2 = rtc_byte_rd(7);
    if(temp == 0xaa && temp2 == 0x10)return;
    
    rtc_byte_wr(0,0);			//Seconds = 0
    rtc_byte_wr(1,24);			//Minutes = 0
    rtc_byte_wr(2,02);		//Hrs = 0, Mode = 12 Hrs (AM/PM mode)
    rtc_byte_wr(4,13);			//Date = 1;
    rtc_byte_wr(5,12);			//Month = 1;
    rtc_byte_wr(6,18);			//Year = 0;
    rtc_byte_wr(7,0x10);		//SQWE
    rtc_byte_wr(63,0xaa);		//signature
    rtc_byte_wr(62,0x01);		//?? to be discarded after checking the use of this address
}

//----------------------------------------------------------------------------//

unsigned char pbcd2hex(unsigned char pbcdbyte)
{
    unsigned char tmp,tmp2;
    
    tmp = pbcdbyte & 0x0f;          //lower nibble
    tmp2 = (pbcdbyte >> 4) & 0x0f;  //upper nibble
    tmp2 *= 10;
    tmp += tmp2;
    return(tmp);
}

unsigned char hex2pbcd(unsigned char hex_num)
{
  char high = 0;
 
  while (hex_num >= 10)                 // Count tens
  {
    high++;
    hex_num -= 10;
  }  
  high = high << 4;
  high &= 0xf0;
  hex_num &= 0x0f;
  high |= hex_num;
  return high;
}

//----------------------------------------------------------------------------//

void poll_rtc(void)
{
    //called every 1mS from basems_wait
    static unsigned int polltmr = 0;
    unsigned char i;
 
    polltmr++;
    if(polltmr < 500)return;
    polltmr = 0;
    
    for(i=0; i<6; i++)
    {
        gen_var.rtc_read[i] = rtc_byte_rd(rtc_addr[i]);
    }
    gen_var.rtc_read[5] &= 0x7f;                        //remove CH flag from seconds
    
    for(i=0; i<6; i++)
    {
        gen_var.rtc_read[i] = pbcd2hex(gen_var.rtc_read[i]);
    }        
}

//----------------------------------------------------------------------------//


//----------------------------------------------------------------------------//

//void update_rtc(void)
//{
//	rtc_byte_wr(0,hex2pbcd(rtc_read[5]));			//Seconds = 0
//	rtc_byte_wr(1,hex2pbcd(rtc_read[4]));			//Minutes = 0
//	rtc_byte_wr(2,hex2pbcd(rtc_read[3]));			//Hrs = 0, Mode = 12 Hrs (AM/PM mode)
//	rtc_byte_wr(4,hex2pbcd(rtc_read[0]));			//Date = 1;
//	rtc_byte_wr(5,hex2pbcd(rtc_read[1]));			//Month = 1;
//	rtc_byte_wr(6,hex2pbcd(rtc_read[2]));			//Year = 0;
//	
//}