#include "main.h"
#include "extern.h"

volatile unsigned int buffer[50];

#define READ_HOLDING		    0x03
#define READ_COIL		    0x01
#define WRITE_COIL		    0x05
#define READ_EXCEPTION		    0x07
#define WRITE_HOLDING		    0x06
#define READ_LOG		    0x08

//----------------------------------------------------------------------------//

unsigned int CRC_16(unsigned char *ptr,unsigned char cnts)
{
    union int_char crcreg;
    unsigned char i;
    crcreg.int_val = 0xffff;
    while(cnts != 0)
    {
	crcreg.char_val[LOWBYTE] ^= *ptr;
	for(i=8; i!=0; i--)
	{
	    if(crcreg.char_val[LOWBYTE] & 0x01 == 1)
	    {	
		crcreg.int_val = crcreg.int_val>>1;
		crcreg.int_val ^= 0xa001;
	    }
	    else
	    {
		crcreg.int_val= crcreg.int_val>>1;
	    }
	}
		
	cnts--;
	ptr++;
    }
    return(crcreg.int_val);
}

//----------------------------------------------------------------------------//

void decode_mbus_frame(void)
{
    unsigned int rec_no;
    union int_char un1;
    unsigned int address,length;
    unsigned int temp;
    
    if(mbus_var.rx_framecmplt_flg == 0)return;
         
    mbus_var.rx_framestart_flg = 0;
    mbus_var.rx_framecmplt_flg = 0;
    mbus_var.rx_tmr = 0;
     
    if(mbus_var.rx_buff[0] != mbus_var.slave_id) 
    {
        mbus_var.ptr = 0;
        return;
    }
    
    temp = CRC_16(mbus_var.rx_buff,mbus_var.ptr-2);       //mbus -2 defines length of data
    un1.char_val[HIGHBYTE] = mbus_var.rx_buff[mbus_var.ptr-1];
    un1.char_val[LOWBYTE]  = mbus_var.rx_buff[mbus_var.ptr-2];
//  un_buffer595[2].bytes = 0xFF;
    
    if(un1.int_val != temp)
    {  
        mbus_var.ptr = 0;
        return;
    }

    un1.char_val[LOWBYTE] = mbus_var.rx_buff[3];
    un1.char_val[HIGHBYTE] = mbus_var.rx_buff[2];
    address=un1.int_val;                                          //read address
    
    un1.char_val[LOWBYTE] = mbus_var.rx_buff[5];
    un1.char_val[HIGHBYTE] = mbus_var.rx_buff[4];
    length=un1.int_val;                                             //read length

    
    switch(mbus_var.rx_buff[1])                                      
    {    	
	case READ_HOLDING   :	// 0x03 READ HOLDING (MULTIPLE BYTE)
				if((address < 25)&&(length < 25))
				{ 
				    send_continuous(length,address);
				}
				else
				    address_error(mbus_var.rx_buff[1]);                                
	break;
        
	case WRITE_HOLDING  :	// 0x06 WRITE HOLDING (MULTIPLE BYTE)
				if(address < 12)
				{ 
				    write_single_holding(length,address);
				}
				else
				    address_error(mbus_var.rx_buff[1]);                                
	break;
	  	 
//      case READ_EXCEPTION	:	// 0x07 READ EXCEPTION STATUS
//                                if ((mbus_rx_buff[2] == 0) && (mbus_rx_buff[3] == 0))
//								{
//									if (length <= 16)
//									{
//										if (mbus_rx_buff[4] == 0xAA)
//										{
//											slave_id = mbus_rx_buff[5];
//											
//										}
//										else
//										{
//										  if (mbus_rx_buff[6] == 0xEE)
//										  {
//											  }
//										  if (mbus_rx_buff[7] == 0xAE)
//										  {
//											rtc_read [0] = mbus_rx_buff[8];
//											rtc_read [1] = mbus_rx_buff[9];
//											rtc_read [2] = mbus_rx_buff[10];
//											rtc_read [3] = mbus_rx_buff[11];
//											rtc_read [4] = mbus_rx_buff[12];
//											rtc_read [5] = mbus_rx_buff[13];
//
//											update_rtc();
//										  }
//										  FillNSend_Echo_Frame();
//										}
//									}
//									else 
//									{
//										address_error(mbus_rx_buff[1]);
//									}
//								}	
//								else 
//								{
//									address_error(mbus_rx_buff[1]);
//								}
//								
//      break;	  
	
	case  READ_LOG      :   rec_no = (int)mbus_var.rx_buff[2];			//Get record no
				rec_no <<= 8;                       
				rec_no += (int)mbus_var.rx_buff[3];
				send_log_rd(rec_no);
//				send_log();
	break;
	  
	default		    :   function_error(mbus_var.rx_buff[1]);
      
	break;            
    }
	
}

//----------------------------------------------------------------------------//

void function_error(unsigned int func)
{    
    unsigned int temp;
    union int_char un1;
    temp = func;
    mbus_var.tx_buff[0] = mbus_var.slave_id;
    mbus_var.tx_buff[1] = temp|0x80;
    mbus_var.tx_buff[2] = 0x01;    //signal function error
    un1.int_val = CRC_16(mbus_var.tx_buff,3);
    mbus_var.tx_buff[3] =  un1.char_val[LOWBYTE];  
    mbus_var.tx_buff[4] = un1.char_val[HIGHBYTE];
    mbus_var.send_frame_cmpr = 4;
    mbus_var.send_ptr = 0;
    mbus_var.frame_send_flag = 1;    
}

//----------------------------------------------------------------------------//

void address_error(unsigned int func)
{

    unsigned int temp;
    union int_char un1;
    temp = func;
    mbus_var.tx_buff[0] = mbus_var.slave_id;
    mbus_var.tx_buff[1] = temp|0x80;
    mbus_var.tx_buff[2] = 0x02;    //signal address error
    un1.int_val = CRC_16(mbus_var.tx_buff,3);
    mbus_var.tx_buff[3] = un1.char_val[LOWBYTE]; 
    mbus_var.tx_buff[4] = un1.char_val[HIGHBYTE];
    mbus_var.send_frame_cmpr = 4;
    mbus_var.send_ptr = 0;
    mbus_var.frame_send_flag = 1;
}

//----------------------------------------------------------------------------//

void FillNSend_Echo_Frame(void)
{
    unsigned char i = 0;
    
    for(i=0;i<mbus_var.ptr;i++)
    {
        mbus_var.tx_buff[i] = mbus_var.rx_buff[i];
    }
    mbus_var.send_frame_cmpr = mbus_var.ptr-1;
    mbus_var.send_ptr = 0;
    mbus_var.ptr = 0;
    mbus_var.frame_send_flag = 1;
}

//----------------------------------------------------------------------------//

void send_to_master(void)
{
    unsigned int i = 0;
	
    if(mbus_var.frame_send_flag == 0)return;
    
    if(mbus_var.send_ptr > mbus_var.send_frame_cmpr)
    {
        mbus_var.send_frame_cmpr = 0;
        mbus_var.frame_send_flag = 0;
        mbus_var.send_ptr = 0;
        mbus_var.ptr = 0;
        return;
    }

//    MAX_485_PIN = 1;   //IF THIS PIN IS HIGH ,TX IS ENABLE
//    while((TX1IF==0)||(TX2IF == 0)){}
//    TX1IF = 0;
    while(TX2IF == 0){}
    TX2IF = 0;
    TXEN2_hbit = 1;
//  while((TX1IF==0)){}
//  TX1IF = 0;
//  TXEN_hbit = 1;
    MAX_485_PIN2  = 1;
    
    TX9D2 = calculate_parity(mbus_var.tx_buff[mbus_var.send_ptr]);
	
    TXREG2 = mbus_var.tx_buff[mbus_var.send_ptr];
    mbus_var.send_ptr++;
    
    for(i=0;i<650;i++)
    {	
        asm("nop");
    }
    MAX_485_PIN2 = 0;
}

//----------------------------------------------------------------------------//

void send_continuous(unsigned int length, unsigned int address)
{

    unsigned char tempc,i,j;
    unsigned int tempi;
    union int_char un1;


    mbus_var.tx_buff[0] = mbus_var.rx_buff[0];
    mbus_var.tx_buff[1] = mbus_var.rx_buff[1];

    tempc = (unsigned char)length;  
    mbus_var.tx_buff[2] = tempc << 1;  // get number of bytes to send

    tempc = (unsigned char)address;
    tempc += length;

    buffer[0] = (int)logic_var.hold_data[0].bytes;
    buffer[1] = (int)logic_var.hold_data[1].bytes;
    buffer[2] = (int)logic_var.hold_data[2].bytes;

    buffer[3] = (int)logic_var.flash_data[0].bytes;
    buffer[4] = (int)logic_var.flash_data[1].bytes;
    buffer[5] = (int)logic_var.flash_data[2].bytes;
	
    buffer[6] = (int)logic_var.new_ip[2];
    buffer[7] = (int)logic_var.new_ip[6];
    buffer[8] = (int)logic_var.new_ip[10];     ///need to change number using 12 inputs 

    buffer[9]  = (int)logic_var.new_ip[1];        // need to change number
    buffer[10] = (int)logic_var.new_ip[5];
    buffer[11] = (int)logic_var.new_ip[9];
	

    buffer[12] = (int)(logic_var.status_flg & 0x8F);
    buffer[13] = (int)logic_var.rly_status;

    buffer[14] = gen_var.log_data_cntr;
    buffer[15] = (int)gen_var.rtc_read [2];//rtc_rd.year;
    buffer[16] = (int)gen_var.rtc_read [0];//rtc_rd.date;
    buffer[17] = (int)gen_var.rtc_read [1];//rtc_rd.month;
    buffer[18] = (int)gen_var.rtc_read [3];//rtc_rd.hour;
    buffer[19] = (int)gen_var.rtc_read [4];//rtc_rd.min;
    buffer[20] = (int)gen_var.rtc_read [5];//rtc_rd.seconds;

    j = 2;
    for (i = address; i < tempc; i++)
    {
        j++;
        mbus_var.tx_buff[j] = buffer[i] >> 8;
        j++;       
        mbus_var.tx_buff[j] = (char)buffer[i];      
    }

    tempi = j+1;
    un1.int_val = CRC_16(mbus_var.tx_buff,tempi);

    j++;
    mbus_var.tx_buff[j] =  un1.char_val[LOWBYTE];
    j++;
    mbus_var.tx_buff[j] =  un1.char_val[HIGHBYTE];

    mbus_var.send_frame_cmpr = j;
    mbus_var.send_ptr = 0;
    mbus_var.frame_send_flag = 1;

}

//----------------------------------------------------------------------------//

void write_single_holding(unsigned int length, unsigned int address)
{
    switch(address)
    {
        case 0  :   if(length == 0X0001)			//SILENCE KEY
		    {
			silence_key();
			FillNSend_Echo_Frame();
		    }	
		    else
			address_error(mbus_var.rx_buff[1]);
        break;
        
        case 1  :   if(length == 0X0002)			//ACK KEY
		    {
			accept_key();
			FillNSend_Echo_Frame();
		    }
		    else
			address_error(mbus_var.rx_buff[1]);
        break;
        
        case 2  :   if(length == 0X0003)			//RESET KEY
		    {
			reset_key();
			FillNSend_Echo_Frame();
		    }
		    else
			address_error(mbus_var.rx_buff[1]);    
        break;
		
	case 4  :   if(length == 0X0004)			//TEST KEY
		    {
			test_key();
			FillNSend_Echo_Frame();
		    }
		    else
			address_error(mbus_var.rx_buff[1]);
    
        break;
		
	case 5	:   if(length == 0xEFEF)
		    {
			gen_var.log_data_cntr = 0;
			log_wr();
			FillNSend_Echo_Frame();
		    }
	break;
		
	case 6  :   gen_var.rtc_read[2] = length ;                  //rtc_wr.year;
		    rtc_byte_wr(6,hex2pbcd(gen_var.rtc_read[2]));
		    FillNSend_Echo_Frame();
	break;
		
	case 7  :   gen_var.rtc_read[1] = length ;                  //rtc_wr.month;;;
		    rtc_byte_wr(5,hex2pbcd(gen_var.rtc_read[1]));
		    FillNSend_Echo_Frame();
	break;
		
	case 8  :   gen_var.rtc_read[0] = length ;                  // rtc_wr.date;
		    rtc_byte_wr(4,hex2pbcd(gen_var.rtc_read[0]));
		    FillNSend_Echo_Frame();
	break;
		
	case 9  :   gen_var.rtc_read[3] = length ;                  //rtc_wr.hour;
		    rtc_byte_wr(2,hex2pbcd(gen_var.rtc_read[3]));
		    FillNSend_Echo_Frame();
	break;
		
	case 10 :   gen_var.rtc_read[4] = length ;                  //rtc_wr.min;
		    rtc_byte_wr(1,hex2pbcd(gen_var.rtc_read[4]));
		    FillNSend_Echo_Frame();
	break;
//		case 11 :	slave_id = length; 
//					if ((slave_id == 0)||(slave_id ==0xff)) 
//							slave_id = 1;
//					i2c_byte_wr(SLAVE_ID_ADDR,slave_id);
//					FillNSend_Echo_Frame();
//		break;
		
	case 11 :   gen_var.log_data_cntr = length;
		    i2c_int_wr(0,LOG_DATA_CNTR_ADDR,gen_var.log_data_cntr);
		    FillNSend_Echo_Frame();
	break;
		
	
		
	}
}

//------------------------------------------------------------------------------

//void send_readonly_byte(unsigned int len)
//{
//    union int_char un1;
//    un1.int_val = len;
//    mbus_tx_buff[0] = 0x01;
//    mbus_tx_buff[1] = 0x04;
//    mbus_tx_buff[2] = un1.char_val[LOWBYTE]<<1;  //byte count
//    un1.int_val = 0x1234;
//    mbus_tx_buff[3] = un1.char_val[HIGHBYTE];
//    mbus_tx_buff[4] = un1.char_val[LOWBYTE];  
//    
//    un1.int_val = 0x6677;
//    mbus_tx_buff[5] = un1.char_val[HIGHBYTE];
//    mbus_tx_buff[6] = un1.char_val[LOWBYTE]; 
//
//    un1.int_val = CRC_16(mbus_tx_buff,7);
//    mbus_tx_buff[7] =  un1.char_val[LOWBYTE];
//    mbus_tx_buff[8] =  un1.char_val[HIGHBYTE];
//    send_frame_cmpr = 8;
//    mbus_send_ptr = 0;
//    frame_send_flag = 1;
//    
//}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//void send_to_master(void)
//{
//    unsigned int i = 0;
//	
//    if(mbus_var.frame_send_flag == 0)return;
//    
//    if(mbus_var.send_ptr > mbus_var.send_frame_cmpr)
//    {
//        mbus_var.send_frame_cmpr = 0;
//        mbus_var.frame_send_flag = 0;
//        mbus_var.send_ptr = 0;
//        mbus_var.ptr = 0;
//        return;
//    }
//
////    MAX_485_PIN = 1;   //IF THIS PIN IS HIGH ,TX IS ENABLE
////    while((TX1IF==0)||(TX2IF == 0)){}
////    TX1IF = 0;
//    while(TX1IF == 0){}
//    TX1IF = 0;
//    TXEN_hbit = 1;
////  while((TX1IF==0)){}
////  TX1IF = 0;
////  TXEN_hbit = 1;
//    MAX_485_PIN1  = 1;
//    
//    TX9D1 = calculate_parity(mbus_var.tx_buff[mbus_var.send_ptr]);
//	
//    TXREG1 = mbus_var.tx_buff[mbus_var.send_ptr];
//    mbus_var.send_ptr++;
//    
//    for(i=0;i<650;i++)
//    {	
//        asm("nop");
//    }
//    MAX_485_PIN1 = 0;
//}

//------------------------------------------------------------------------------

//************************************************************************
