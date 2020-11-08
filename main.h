



#include <xc.h>

#define TRUE                        1
#define FALSE                       0

#define TRUE_                       0
#define FALSE_                      1

#define LOWBYTE                     0
#define HIGHBYTE                    1

#define main_hooter_hbit_           LATBbits.LATB4
#define second_hooter_hbit_         LATBbits.LATB5  
#define third_hooter_hbit_          LATAbits.LATA0 

#define HEALTH_FLASH_LED            LATBbits.LATB7 

#define FLASH_DISPLAY_TIMER         500                                         //RANGE 0 TO 65535

#define CLK_595_RELAY               LATAbits.LATA6
#define DATA_595_RELAY              LATEbits.LATE2 
#define STB_595_RELAY               LATAbits.LATA7
#define OE_595_RELAY                LATEbits.LATE1
           
#define CLK_595_LEDS                LATCbits.LATC0 
#define DATA_595_LEDS               LATCbits.LATC2
#define STB_595_LEDS                LATCbits.LATC1
           
#define TEST_KEY                    PORTDbits.RD0 
#define SILENCE_KEY                 PORTDbits.RD1
#define ACK_KEY                     PORTDbits.RD2 
#define RESET_KEY                   PORTDbits.RD3 
           
#define CLK_165_FAULT               LATAbits.LATA2
#define DATA_165_FAULT              PORTAbits.RA3
#define STB_165_FAULT               LATAbits.LATA1
           
#define CLK_165_DIP                 LATAbits.LATA4
#define DATA_165_DIP                PORTEbits.RE0
#define STB_165_DIP                 LATAbits.LATA5


#define TXEN_hbit                   LATCbits.LATC6
#define MAX_485_PIN1                LATCbits.LATC5
#define TXEN2_hbit                  LATDbits.LATD6
#define MAX_485_PIN2                LATDbits.LATD5

#define UART_SWITCHER               PORTDbits.RD4 

#define	sda_hbit                    PORTCbits.RC4
#define	scl_hbit                    PORTCbits.RC3

#define	SDA_TRISIN                  TRISCbits.TRISC4 = 1
#define	SDA_TRISOUT                 TRISCbits.TRISC4 = 0

#define	SDA_TRIS                    TRISCbits.TRISC4
#define	SCL_TRIS                    TRISCbits.TRISC3

#define	I2C_WRDELAY_CONST           12                                          //If systick time is 1mS.

#define LOG_DATA_CNTR_ADDR          1

//#define SWITCH                    PORTDbits.RD4

union BitByte
{
	struct
	{
		unsigned bit0:1;
		unsigned bit1:1;
		unsigned bit2:1;
		unsigned bit3:1;
		unsigned bit4:1;
		unsigned bit5:1;
		unsigned bit6:1;
        unsigned bit7:1;
    };
    unsigned char bytes;
};

union int_char
{
	unsigned int int_val;
	unsigned char char_val[2];
};

struct general_purpose 
{
    unsigned char basems_flg;
    unsigned int health_cntr;
    unsigned char isr_cntr;
    unsigned char i2c_wr_cntr;
    unsigned char rtc_read[6];
    unsigned int log_data_cntr;
    unsigned char logevent_flg;
    union BitByte loghist_data[3];
};

struct logic_purpose
{
//    unsigned char input_buffer_165[12][8];
    unsigned char input_buffer_165[12];                                         //used to catch 165 raw data
    union BitByte input_array[12];                                              //used to store all latch and switch read data without noise rejection
    unsigned char input_hist_array[12];
    unsigned char new_ip[12];

    union BitByte hold_data[3];
    union BitByte hist_hold_data[3];                                            //used for copy hold_data after after press accept and reset key
    union BitByte flash_data[3];
    union BitByte cal_data[3];    
    union BitByte silence_hist[3];
    
    unsigned int flash_timer;
    unsigned char flash_flg;
    union BitByte repeat_rly[3];                                                //
    union BitByte led_byte[6];                  
    union BitByte key_data;
    union BitByte rly_data[3];
    unsigned char status_flg;
    unsigned char rly_status;
    unsigned char sure_ip_cntr1,sure_ip_cntr2;
};

struct modbus_purpose
{
    unsigned char rx_tmr;
    unsigned char tx_buff[50];
    unsigned char rx_buff[50];
    unsigned char send_frame_cmpr;
    unsigned char rx_framecmplt_flg;
    unsigned char rx_framestart_flg;
    unsigned char frame_send_flag;
    unsigned char send_ptr,ptr;
    unsigned char slave_id;
};

//================================data_log.c==================================//

void datalog_event_check(void);
void log_wr(void);
void send_log_rd(unsigned int record);

//===============================hw_layer.c===================================//

void update_595(void);
void update_repeat_relay(void);
void read_165_fault(void);
void read_165_dip(void);
void send_byte(unsigned char send_byte);
void hex2asc(unsigned char n);
void bcd_array_ascii(unsigned char c);
void clr_bcd_array(void);
void int2bcd_4dig(unsigned int intval);
void intval_on_pc(unsigned int uitemp);
void char2bcd_3dig(unsigned char charval);
void char_val_pc(unsigned char val);
void dicing_slicing(void);
unsigned char calculate_parity(unsigned char val);


//void test_fault(void);
//void dance_leds(void);

//==============================i2c_e2p_rtc.c=================================//

void i2c_delay(void);
void i2c_shift_out(unsigned char dat, unsigned char cnts);
unsigned char i2c_shift_in(unsigned char count);
void i2c_ackout(void);
void i2c_no_ackout(void);
void i2c_start(void);
void i2c_stop(void);
unsigned char i2c_byte_rd(unsigned char dev_addr,unsigned long adr);
unsigned int i2c_int_rd(unsigned char dev_addr,unsigned long adr);
void init_i2c_gpios(void);
void i2c_byte_wr(unsigned char dev_addr,unsigned long adr,unsigned char dta);
void i2c_int_wr(unsigned char dev_addr,unsigned long adr,unsigned int dta);

//---------------------------------RTC----------------------------------------//

void rtc_byte_wr(unsigned char adr,unsigned char dta);
unsigned char rtc_byte_rd(unsigned char adr);
void init_rtc(void);
unsigned char pbcd2hex(unsigned char pbcdbyte);
unsigned char hex2pbcd(unsigned char hex_num);
void poll_rtc(void);

//==================================logic.c===================================//

void noise_rejection_off(void);
void test_key(void);
void silence_key(void);
void accept_key(void);
void reset_key(void);
void fault_detection(void);

//==================================main.c====================================//

void health_flash(void);
void init(void);
void timer_init(void);
void init_uart(void);
void delay_ms(unsigned int ms);
void basems_wait(void);

//===============================modbus.c=====================================//

unsigned int CRC_16(unsigned char *ptr,unsigned char cnts);
void decode_mbus_frame(void);
void function_error(unsigned int func);
void address_error(unsigned int func);
void FillNSend_Echo_Frame(void);
void send_to_master(void);
void send_continuous(unsigned int length, unsigned int address);
void write_single_holding(unsigned int length, unsigned int address);

