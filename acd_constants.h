/* ************************************************************ */
/* *** CONSTANT DEFINITIONS *********************************** */
/* ************************************************************ */
// Define Slave I2C Address
#define SLAVE_ADDR 0x12

// Define Slave answer size
#define ANSWERSIZE 6

/* Definition of test string constants */

#define COMPANY "AC DEVELOPMENTS"
#define MODEL   "... eMonitor ..."
#define SERIAL_NO "Ser No: 00000001"
#define VERSION   "Vers:   A.4.27"
#define BY      "by:"
#define AUTHOR  "Dr A Collinson"
#define INITIAL1 "Initialising:"
#define INITIAL2 "Please Wait ...>"

/* definition of the EEPROM ADDRESSES (and the set/reset flag bytes) */

#define SET_FLAG          0x0
#define RESET_FLAG        0x1
#define EE_MENU           0x10
#define EE_CT             0x11
#define EE_BURDEN         0x12
#define EE_SAMPLE         0x13
#define EE_ILLUM          0x14
#define EE_VREF           0x15
#define EE_VMAINS         0x16
#define EE_LOG            0x17
#define EE_RESTORE        0x18

/* definition of the parameter array pointers */

#define MENU_PARAM    0
#define CT_PARAM      1
#define BURDEN_PARAM  2
#define SAMPLE_PARAM  3
#define ILLUM_PARAM   4
#define VREF_PARAM    5
#define VMAINS_PARAM  6
#define LOG_PARAM     7
#define RESTORE_PARAM 8
#define CALIBRAT      9

/* Definition of iniital parameter setting indexes */

#define INITIAL_MENU    0
#define INITIAL_CT      2
#define INITIAL_BURDEN  3
#define INITIAL_SAMPLE  1
#define INITIAL_ILLUM   8
#define INITIAL_VREF    2
#define INITIAL_VMAINS  6
#define INITIAL_LOG     1

/* Definition of useful constants */

#define MAX_BRIGHTNESS 0
#define MIN_BRIGHTNESS 45
#define ADC_RES     1023            // ADC resolution
#define DISPLAYTIME 2000
#define LOOPTIME    200
#define BAUD_RATE   9600
#define INPUTPIN    A3              // ' hot' Analog input pin (current measurement)
#define NEUTRALPIN  A1              // 'cold' Analog input pin (current measurement)
#define LOG_OFF  0                  // flag to close the log file 
#define LOG_ON   1                  // flag to open the log file
#define NUL         0x00
#define CMD_0       0x00
#define CMD_1       0x01

/* LCD Screen size */
#define COLUMN  16
#define ROW     2

/* Allocation of the NANO data control pins */

/* Serial communications */
#define SERIAL_RX   0
#define SERIAL_TX   1

/* External Interrupt */
#define EXT_INT     2

/* PWM Digital output */
#define PWM_OUT     3

/* LCD Conteol and data pins */
#define RS      4
#define EN      5
#define D4      6
#define D5      7
#define D6      8
#define D7      9
 
/* Rotary Encoder Inputs */
#define input_CLK 10
#define input_DT 11
#define input_SW 12

/* On-board LED */
#define ledPin      13              // LED pin

/* Shaft Encoder debounce */ 
 #define DEBOUNCE  80

/* Menu constants - number of items in each option list */ 

 #define MIN_COUNT    0
 #define MENU_COUNT   9
 #define CT_COUNT     6    
 #define BURDEN_COUNT 6
 #define SAMPLE_COUNT 6
 #define ILLUM_COUNT  11
 #define VREF_COUNT   8
 #define VMAINS_COUNT 10
 #define LOG_COUNT    2
 #define RESTORE_COUNT 2
