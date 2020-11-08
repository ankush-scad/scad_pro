#include "main.h"
#include"extern.h"
unsigned char chip_address;  

//----------------------------------------------------------------------------//

void datalog_event_check(void)
{
    unsigned char i;
    for(i=0;i<3;i++)
    {
	if (logic_var.flash_data[i].bytes != 0)
	{
	    if(gen_var.loghist_data[i].bytes != logic_var.flash_data[i].bytes)     //in logevent_data store  new input log value
	    {
		gen_var.loghist_data[i].bytes = logic_var.flash_data[i].bytes;
		gen_var.logevent_flg = 1;
	    }
	}
    }
}

//----------------------------------------------------------------------------//

void log_wr(void)
{

  unsigned long templ,chip_start;
  
  unsigned char i;
  unsigned char log_buffer[20];
  
  gen_var.log_data_cntr++;
  i2c_int_wr(0,LOG_DATA_CNTR_ADDR,gen_var.log_data_cntr);

  templ = (long)(gen_var.log_data_cntr * 16) + 16;
  
  if (templ <= 0x0ffff)               //looging address starts after 0x2 first e2p
    {
      chip_address = 0;
      chip_start = templ;
    }
    else if (templ <= 0x1ffff)
    {
      chip_address = 0; 
      chip_start = templ;
    }                                       //FIRST E2P COMPLETED 
    else if (templ <= 0x2ffff)
    {
      chip_address = 1;   
      chip_start = templ - 0x20000;  
    }
    else if (templ <= 0x3ffff)          
    {
      chip_address = 1; 
      chip_start = templ - 0x20000;   
    }                                       //SECOND E2P COMPLETED 
    else if (templ <= 0x4ffff)
    {
      chip_address = 2;   
      chip_start = templ - 0x40000;  
    }
    else if (templ <= 0x5ffff) 
    {
      chip_address = 2; 
      chip_start = templ - 0x40000;   
    }                                       //THIRD E2P COMPLETED 
    else if (templ <= 0x6ffff)
    {
      chip_address = 3;   
      chip_start = templ - 0x60000;  
    }
    else if (templ <= 0x7ffff)
    {
      chip_address = 3; 
      chip_start = templ - 0x60000;   
    }                                       //FOURTH E2P COMPLETED 
     
    log_buffer[0] = gen_var.rtc_read[0];//date;
    log_buffer[1] = gen_var.rtc_read[1];//month;
    log_buffer[2] = gen_var.rtc_read[2];//year;
    log_buffer[3] = gen_var.rtc_read[3];//hour;
    log_buffer[4] = gen_var.rtc_read[4];//min;
    log_buffer[5] = gen_var.rtc_read[5];//seconds;
  
    log_buffer[6] = logic_var.status_flg;
  
    log_buffer[7] = logic_var.flash_data[0].bytes;
    log_buffer[8] = logic_var.flash_data[1].bytes;
    log_buffer[9] = logic_var.flash_data[2].bytes;

    log_buffer[10] = logic_var.new_ip[2];                               // relay group status
    log_buffer[11] = logic_var.new_ip[6];
    log_buffer[12] = logic_var.new_ip[10];
  
    log_buffer[13] = logic_var.new_ip[1];                               //  NO/NC
    log_buffer[14] = logic_var.new_ip[5];
    log_buffer[15] = logic_var.new_ip[9];

    for(i=0; i<16; i++)
    {   
        i2c_byte_wr(chip_address,(long)chip_start, log_buffer[i]);
	chip_start++;
    }   
    gen_var.logevent_flg = 0;
}

//----------------------------------------------------------------------------//

void send_log_rd(unsigned int record)
{
    unsigned long templ;
    unsigned int tempi; 
    unsigned char i;

    templ = (long)(record * 16) + 16;

    mbus_var.tx_buff[0] =  mbus_var.slave_id;
    mbus_var.tx_buff[1] =  8;   // Non Modbus Function Code for Log Read
    mbus_var.tx_buff[2] = (char)(record  >> 8);
    mbus_var.tx_buff[3] = (char)(record);

    for(i=0;i<16;i++)
    {
        mbus_var.tx_buff[i+4] = i2c_byte_rd(chip_address,templ);
        templ++;
    }

    tempi = CRC_16(mbus_var.tx_buff,20);
    mbus_var.tx_buff[20] = tempi >> 8;        //msb of crc
    mbus_var.tx_buff[21] = (char)tempi;       //lsb of crc

    mbus_var.send_frame_cmpr = 21;
    mbus_var.frame_send_flag = 1;
    mbus_var.send_ptr = 0;
  
}

