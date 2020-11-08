#include "main.h"
#include "extern.h"

//const unsigned char red_led[8] = {0x02, 0x08, 0x10, 0x40, 0x02, 0x08, 0x10, 0x40};
//const unsigned char green_led[8] = {0x01, 0x04, 0x20, 0x80, 0x01, 0x04, 0x20, 0x80};
//const unsigned char onebit_low_tbl[8] =  {0xfe,0xfd,0xfb,0xf7,0xef,0xdf,0xbf,0x7f};
const unsigned char onebit_high_tbl[8] =  {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};
//const unsigned char rep_rly_tbl[8] =  {0xfb,0xfd,0xfe,0xf7,0xef,0xdf,0x7f,0xbf};
const unsigned char red_led_tbl[2] = {2,0};
//const unsigned char red_led_tbl[3] = {4,2,0};
const unsigned char rep_rly_tbl[8] =  {0x04,0x02,0x01,0x08,0x10,0x20,0x80,0x40};
const unsigned char no_nc_tbl[3] = {3,5,7};
const unsigned char group_tbl[3] = {4,6,8};

//----------------------------------------------------------------------------//

void noise_rejection_off(void)
{
    unsigned char temp1,temp2,new_data,hist_data;
    unsigned char i;
    
    for(i = 0; i < 9; i++)
    {
        new_data = logic_var.input_array[i].bytes;     //input sense on 1's
        hist_data = logic_var.input_hist_array[i];
        temp1 = ~ (new_data ^ hist_data);
        temp2 =~ temp1; 
        logic_var.new_ip[i] |= (temp2 & new_data); //getting sure 1's 
        logic_var.new_ip[i] &= (temp1 | new_data); //getting sure 0's
        logic_var.input_hist_array[i] = new_data;
    }
}

//----------------------------------------------------------------------------//

void test_key(void)
{
    unsigned char i;
    logic_var.status_flg |= 0x01;
    for(i = 0; i < 3; i++)
    {
        logic_var.hist_hold_data[i].bytes = 0;
        logic_var.hold_data[i].bytes = 0xff;
//        logic_var.rly_data[i].bytes = 0xff;
//	logic_var.repeat_rly[i].bytes = 0;
    }
    
    logic_var.rly_status |= 0x03;
    main_hooter_hbit_ = TRUE_;
    second_hooter_hbit_ = TRUE_;

}

//----------------------------------------------------------------------------//

void silence_key(void)
{
    unsigned char i;
//    silent_flag =1;
    logic_var.status_flg|=0x08;
    for(i = 0; i < 3; i++)
    {
        logic_var.silence_hist[i].bytes = logic_var.hold_data[i].bytes;
    }
    logic_var.rly_status &= ~0x03;
    main_hooter_hbit_ = FALSE_;
    second_hooter_hbit_ = FALSE_;    

}

//----------------------------------------------------------------------------//

void accept_key(void)
{
    unsigned char i;
    if((logic_var.status_flg & 0x08) == 0x08)
    {
	for(i = 0; i < 3; i++)
	{
	    logic_var.flash_data[i].bytes = 0;
	    logic_var.hist_hold_data[i].bytes = logic_var.hold_data[i].bytes;
	}
	logic_var.rly_status &= ~0x03;
	logic_var.status_flg |= 0x04;        
    }
}

//----------------------------------------------------------------------------//

void reset_key(void)
{
    unsigned char i;
	
    if((logic_var.status_flg & 0x04) == 0x04)
    {      
	for(i = 0; i < 3; i++)
	{
	    logic_var.hold_data[i].bytes = 0;
	    logic_var.flash_data[i].bytes = 0;
	    logic_var.hist_hold_data[i].bytes = 0;
	    logic_var.silence_hist[i].bytes = 0;
	    gen_var.loghist_data[i].bytes = 0;
	    
	}
	logic_var.status_flg &= ~0x0d;
	logic_var.rly_status &= ~0x03;
       
//	if(test_flag == 1)reset_flag = 1;
//        
//        test_flag = 0;
//        silent_flag = 0;
	}
}

//----------------------------------------------------------------------------//

void fault_detection(void)
{
    unsigned char i,k;
//  unsigned char temp, led_data[3], red_led_data[3], green_led_data[3], j,l;
    unsigned char temp, led_data, red_led_data, green_led_data, j,l,rly_data;
    unsigned char input = 0;
    unsigned char dipsw = 0;
    unsigned char tmp = 0,tmp2 = 0,tmp3 = 0;
     
    for(i=0;i<2;i++)
    {       
        temp = ~logic_var.new_ip[i];						//after noise rejection new_input array data invereted because all latch normally gives 0xff
        logic_var.cal_data[i].bytes = (~( temp ^ (logic_var.new_ip[no_nc_tbl[i]])));	//logic 0 or 1 switch check
        logic_var.hold_data[i].bytes |= logic_var.cal_data[i].bytes;		//if new inputs are coming that add to hold_data 

        logic_var.flash_data[i].bytes = logic_var.hold_data[i].bytes ^ logic_var.hist_hold_data[i].bytes;    //in flash_data store flash lamp value
		
        if(logic_var.flash_timer == 0)                                               //flash timer increment every 1msec 
        {
            logic_var.flash_flg = ~logic_var.flash_flg;
            logic_var.flash_timer = FLASH_DISPLAY_TIMER;
        }
	
	if(logic_var.flash_flg == 0)                                                                    //for flash lamp	   
    {
        led_data = logic_var.hold_data[i].bytes & (~logic_var.flash_data[i].bytes);            //off flash lamp for 500msec	  
        red_led_data = (led_data & logic_var.new_ip[i+1]);
	    green_led_data = (led_data & (~logic_var.new_ip[i+1]));
    }
	else 
	    led_data = logic_var.hold_data[i].bytes;	   

    	
	if(i == 0)
	{
	    for(j=0; j<4; j++)
	    {
		if(led_data & onebit_high_tbl[j])
		{		
		    logic_var.led_byte[red_led_tbl[i]].bytes |= onebit_high_tbl[3-j];
		}
		else
		    logic_var.led_byte[red_led_tbl[i]].bytes &= ~onebit_high_tbl[3-j];
	    }
	    for(j=0; j<4; j++)
	    {
		if(led_data & onebit_high_tbl[j+4])
		{		
		    logic_var.led_byte[red_led_tbl[i+1]].bytes |= onebit_high_tbl[7-j];
		}
		else
		    logic_var.led_byte[red_led_tbl[i+1]].bytes &= ~onebit_high_tbl[7-j];
	    }
	    
        }
	else
	{
	    for(j=0; j<4; j++)
	    {
		if(led_data & onebit_high_tbl[j])
		{		
		    logic_var.led_byte[red_led_tbl[i]].bytes |= onebit_high_tbl [3-j];
		}
		else
		    logic_var.led_byte[red_led_tbl[i]].bytes &= ~onebit_high_tbl[3-j];
	    }	    	    	   
	}
        logic_var.repeat_rly[i].bytes = ~logic_var.hold_data[i].bytes;	    
    }    		    	 

    
    if(TEST_KEY  == 0)
	test_key();
    else if(SILENCE_KEY == 0)
	silence_key();
    else if (ACK_KEY == 0)
	accept_key();
    else if (RESET_KEY == 0)
	reset_key();
    
    
    for(i=0;i<2;i++)
    {
	if(i == 0)	    
	{
	    dipsw = ~logic_var.new_ip[group_tbl[i]];                            //get dipsw status (active high)
	    tmp2 = logic_var.silence_hist[i].bytes;
	    tmp3 = logic_var.hold_data[i].bytes;
	}
	else
	{
	    dipsw = ~(logic_var.new_ip[group_tbl[i]]& 0x0f);                            //get dipsw status (active high)
	    tmp2 = logic_var.silence_hist[i].bytes & 0x0f;
	    tmp3 = logic_var.hold_data[i].bytes & 0x0f;
	}
//        input = logic_var.silence_hist[i].bytes ^ logic_var.hold_data[i].bytes;       //obtain only unsilenced one's
	
//	if(input != 0)
//	{
//	    main_hooter_hbit_ = TRUE_;
//	    second_hooter_hbit_ = TRUE_;	    
//	}
//	    
//	if(i == 0)	    
//	    dipsw = ~logic_var.new_ip[group_tbl[i]];                            //get dipsw status (active high)
//	else
//	    dipsw = ~(logic_var.new_ip[group_tbl[i]]& 0x0f);                            //get dipsw status (active high)
	
	
	input = tmp2 ^ tmp3;
        tmp = input ^ dipsw;
	
//        if((tmp & input) != 0)
	if(((~tmp) & input) != 0)
        {	    
//	    if(sure_input_cntr1 == 5)
	    {
		main_hooter_hbit_ = TRUE_;
		logic_var.rly_status |= 0x01;
//		sure_input_cntr1 = 0;
	    }
//	    else
//		sure_input_cntr1++;
        }
//	else
//	    sure_input_cntr1 = 0;
        
//	if(((~tmp) & input) != 0)
	if((tmp & input) != 0)
        {
//	    if(sure_input_cntr2 == 5)
	    {
		second_hooter_hbit_ = TRUE_;	    
		logic_var.rly_status |= 0x02;
//		sure_input_cntr2 = 0;
	    }
//	    else
//		sure_input_cntr2++;
        }
//	else
//	    sure_input_cntr2 = 0;
    }
    
}

