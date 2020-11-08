#include "main.h" 

//-----------------------------Configuration----------------------------------//
// PIC18F46K22 Configuration Bit Settings

// 'C' source line config statements


// CONFIG1H
#pragma config FOSC = INTIO67        // Oscillator Selection bits (XT oscillator)
#pragma config PLLCFG = OFF        // 4X PLL Enable (Oscillator used directly)
#pragma config PRICLKEN = ON      // Primary clock enable bit (Primary clock is always enabled)
#pragma config FCMEN = OFF       // Fail-Safe Clock Monitor Enable bit (Fail-Safe Clock Monitor disabled)
#pragma config IESO = OFF         // Internal/External Oscillator Switchover bit (Oscillator Switchover mode disabled)

// CONFIG2L
#pragma config PWRTEN = ON     // Power-up Timer Enable bit (Power up timer disabled)
#pragma config BOREN = SBORDIS  // Brown-out Reset Enable bits (Brown-out Reset enabled in hardware only (SBOREN is disabled))
#pragma config BORV = 190       // Brown Out Reset Voltage bits (VBOR set to 1.90 V nominal)

// CONFIG2H
#pragma config WDTEN = OFF      // Watchdog Timer Enable bits (Watch dog timer is always disabled. SWDTEN has no effect.)
#pragma config WDTPS = 32768    // Watchdog Timer Postscale Select bits (1:32768)

// CONFIG3H
#pragma config CCP2MX = PORTC1  // CCP2 MUX bit (CCP2 input/output is multiplexed with RC1)
#pragma config PBADEN = ON     // PORTB A/D Enable bit (PORTB<5:0> pins are configured as digital I/O on Reset)
#pragma config CCP3MX = PORTB5  // P3A/CCP3 Mux bit (P3A/CCP3 input/output is multiplexed with RB5)
#pragma config HFOFST = ON     // HFINTOSC Fast Start-up (HFINTOSC output and ready status are delayed by the oscillator stable status)
#pragma config T3CMX = PORTC0   // Timer3 Clock input mux bit (T3CKI is on RC0)
#pragma config P2BMX = PORTD2   // ECCP2 B output mux bit (P2B is on RD2)
#pragma config MCLRE = EXTMCLR  // MCLR Pin Enable bit (MCLR pin enabled, RE3 input pin disabled)

// CONFIG4L
#pragma config STVREN = ON     // Stack Full/Underflow Reset Enable bit (Stack full/underflow will not cause Reset)
#pragma config LVP = OFF        // Single-Supply ICSP Enable bit (Single-Supply ICSP disabled)
#pragma config XINST = OFF      // Extended Instruction Set Enable bit (Instruction set extension and Indexed Addressing mode disabled (Legacy mode))

// CONFIG5L
#pragma config CP0 = ON        // Code Protection Block 0 (Block 0 (000800-003FFFh) not code-protected)
#pragma config CP1 = ON        // Code Protection Block 1 (Block 1 (004000-007FFFh) not code-protected)
#pragma config CP2 = ON        // Code Protection Block 2 (Block 2 (008000-00BFFFh) not code-protected)
#pragma config CP3 = ON        // Code Protection Block 3 (Block 3 (00C000-00FFFFh) not code-protected)

// CONFIG5H
#pragma config CPB = ON        // Boot Block Code Protection bit (Boot block (000000-0007FFh) not code-protected)
#pragma config CPD = OFF       // Data EEPROM Code Protection bit (Data EEPROM not code-protected)

// CONFIG6L
#pragma config WRT0 = OFF       // Write Protection Block 0 (Block 0 (000800-003FFFh) not write-protected)
#pragma config WRT1 = OFF       // Write Protection Block 1 (Block 1 (004000-007FFFh) not write-protected)
#pragma config WRT2 = OFF       // Write Protection Block 2 (Block 2 (008000-00BFFFh) not write-protected)
#pragma config WRT3 = OFF       // Write Protection Block 3 (Block 3 (00C000-00FFFFh) not write-protected)

// CONFIG6H
#pragma config WRTC = OFF       // Configuration Register Write Protection bit (Configuration registers (300000-3000FFh) not write-protected)
#pragma config WRTB = OFF       // Boot Block Write Protection bit (Boot Block (000000-0007FFh) not write-protected)
#pragma config WRTD = OFF       // Data EEPROM Write Protection bit (Data EEPROM not write-protected)

// CONFIG7L
#pragma config EBTR0 = OFF      // Table Read Protection Block 0 (Block 0 (000800-003FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR1 = OFF      // Table Read Protection Block 1 (Block 1 (004000-007FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR2 = OFF      // Table Read Protection Block 2 (Block 2 (008000-00BFFFh) not protected from table reads executed in other blocks)
#pragma config EBTR3 = OFF      // Table Read Protection Block 3 (Block 3 (00C000-00FFFFh) not protected from table reads executed in other blocks)

// CONFIG7H
#pragma config EBTRB = OFF      // Boot Block Table Read Protection bit (Boot Block (000000-0007FFh) not protected from table reads executed in other blocks)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.
//----------------------------Structure_Variable------------------------------//

struct general_purpose gen_var;
struct logic_purpose logic_var;
struct modbus_purpose mbus_var;

unsigned bcd_array[8];

unsigned char sure_input_cntr1 = 0;
unsigned char sure_input_cntr2 = 0;

unsigned char uart_data = 0 ;

//----------------------------------------------------------------------------//

void health_flash(void)
{
    gen_var.health_cntr++;
    if(gen_var.health_cntr != 250)  return;
    gen_var.health_cntr = 0;
    HEALTH_FLASH_LED = ~HEALTH_FLASH_LED;
}

//----------------------------------------------------------------------------//

static void interrupt isr(void)
{
//Interrupt occurring every 1mSec.
    GIE = 0;
    if(TMR1IF == 1)
    {
        TMR1IF = 0;
        TMR1H = 0XF0;         									
        TMR1L = 0x5F;         	    
        gen_var.basems_flg = 1;
	health_flash();	
	
	if(gen_var.i2c_wr_cntr != 0)
	    gen_var.i2c_wr_cntr--;
	
	if(logic_var.flash_timer != 0)
	    logic_var.flash_timer--;	
	
	if(mbus_var.rx_framestart_flg == 1)
        {   
            if(mbus_var.rx_tmr < 5)  mbus_var.rx_tmr++;
        }
        else mbus_var.rx_tmr = 0;
    
        if(mbus_var.rx_tmr == 5)
        {
            mbus_var.rx_framestart_flg = 0;
            mbus_var.rx_framecmplt_flg = 1;
        }        	
    }
    if(RC2IF == 1)
    {	
	RC2IF = 0;   
//	uart_data = RCREG2;
	mbus_var.rx_buff[mbus_var.ptr] = RCREG2;
        mbus_var.ptr++;
        mbus_var.rx_framestart_flg = 1;
        mbus_var.rx_tmr = 0;
    }
//    if(RC1IF == 1)
//    {	
//	RC1IF = 0;   
////	uart_data = RCREG2;
//	mbus_var.rx_buff[mbus_var.ptr] = RCREG1;
//        mbus_var.ptr++;
//        mbus_var.rx_framestart_flg = 1;
//        mbus_var.rx_tmr = 0;
//    }
    
    GIE = 1;	
}

//----------------------------------------------------------------------------//

void init(void)
{
    TRISA = 0X08;    
    TRISB = 0X00;
    TRISC = 0X00;
    PORTC = 0X07;
    TRISD = 0X0F;
    TRISE = 0X02;
//    TRISE = 0X00;       //24-7-19
    
    PORTA = 0X00;
    PORTB = 0X00;
    
    PORTD = 0X00;
    PORTE = 0X00;
    
    ADCON0 = 0;
    ADCON1 = 0;
    
    ANSELA = 0;
    ANSELB = 0;

    ANSELC = 0;
    ANSELD = 0;
    ANSELE = 0;
    CCP3CON = 0x00;
    CCP1CON = 0x00;
    CCP2CON = 0x00;
	
    OSCCONbits.IRCF = 7;
    OSCCONbits.SCS = 3;
//    un_repeat_relay[0].bytes = 0xFF;
    
}

//----------------------------------------------------------------------------//

void timer_init(void)
{
    TMR1IE = 1;
    PEIE = 1;
    GIE = 1;
    TMR1IF = 0;
    TMR1H = 0xF0;    	//	1 MILI SEC.								
    TMR1L = 0X5F; 																		   
//    TMR1H = 0xF8;    	//	1 MILI SEC.								
//    TMR1L = 0X2F; 																		   
    T1CON = 0x01;    
}

//----------------------------------------------------------------------------//

void init_uart(void)
{
    TRISC7 = 1;  //RX
    TRISC6 = 0;  //TX
    TRISD7 = 1;  //RX
    TRISD6 = 0;  //TX
//    UART_SWITCHER = 1;     //1 for normal uart 0 for MODBUS RC6 RC7
    UART_SWITCHER = 0;     //1 for normal uart 0 for MODBUS RC6 RC7
    SYNC1 = 0;
    SYNC2 = 0;
    BRG161 = 0;  //'0' MEANS 8 BIT BAUD RATE GENRATER & 1 MEANS 16 BIT BAUD RATE GENRATER
    BRG162 = 0;
    BRGH1 = 1; 	// THIS BIT IS IN TXSTA,0 MEANCE LOW SPEED  &VICE VARSA
    BRGH2 = 1;
    
    SPBRG1 = 103; 												
    SPBRGH1 = 0;
    SPBRG2 = 103;
    SPBRGH2 = 0;
    
    TX92 = 0;
    RX92 = 0;
    
    TX1STA = 0x24;
    TX2STA = 0x24;
    TXEN1 = 1;	
    TXEN2 = 1;
    RC1IE = 1;
    RC2IE = 1 ;
    GIE =1; PEIE =1;
    RC1STA = 0x90;
    RC2STA = 0x90;   
    RC1IF = 0;
    RC2IF = 0;
}

//----------------------------------------------------------------------------//

void delay_ms(unsigned int ms)
{
    while(ms > 0)
    {
	ms--;
        basems_wait();
    }
}

//----------------------------------------------------------------------------//

void basems_wait(void)
{
    while(1)
    {
	gen_var.basems_flg = 0;
	while(gen_var.basems_flg == 0){}
	gen_var.basems_flg = 0;
	update_595();
	gen_var.isr_cntr++;
	switch(gen_var.isr_cntr)
	{
	    case 1  :	update_595();
	    break;		
	    	   
	    case 2  :	read_165_fault();
	    break;
        
	    case 3  :   read_165_dip();
	    break;
	    
	    case 4  :	dicing_slicing();					
			noise_rejection_off();
	    break;
        
	    case 5  :   update_repeat_relay();
	    break;
	    
	    case 6  :	poll_rtc();
	    break;
	    
	    case 7  :	decode_mbus_frame();		
	    break;	    
	    
	    case 8  :	send_to_master();			
	    break;
        
	    case 9  :	gen_var.isr_cntr = 0;
	    return;	    	    
	}	
    }       
}

//----------------------------------------------------------------------------//

void main(void) 
{    
    init();
    timer_init();
    asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
    TRISE1=0;
//    while(1)
//    {
////        basems_wait();
//        STB_165_DIP = 1;
//        DATA_165_DIP = 1;
//        CLK_165_DIP = 1;
//  //      delay_ms(250);
//        asm("nop"); asm("nop"); asm("nop");
//        asm("nop"); asm("nop"); asm("nop");
//        STB_165_DIP = 0;
//        DATA_165_DIP = 0;
//        CLK_165_DIP = 0;
//    //      delay_ms(250);
//        asm("nop"); asm("nop"); asm("nop");
//        asm("nop"); asm("nop"); asm("nop");
//    }
    main_hooter_hbit_ = FALSE_;
    second_hooter_hbit_ = FALSE_;
    //24-7-19
    logic_var.repeat_rly[0].bytes=0xFF;
    logic_var.repeat_rly[1].bytes=0xFF;
    logic_var.repeat_rly[2].bytes=0xFF;
    
    OE_595_RELAY = 1;         //OUTPUT DISABLE
    delay_ms(100);
    OE_595_RELAY  = 0;         //OUTPUT ENABLE     
    init_uart();
    init_i2c_gpios();
    
    mbus_var.slave_id = 1;
    gen_var.log_data_cntr = i2c_byte_rd(0,LOG_DATA_CNTR_ADDR);    
    init_rtc();
    //24-7-19//test
    
    
    while(1)
    {	
	basems_wait();
	fault_detection();			
		
	datalog_event_check();
	if(gen_var.logevent_flg == 1)
	    log_wr();			   
    }
}


