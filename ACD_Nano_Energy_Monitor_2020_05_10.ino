 /*
  * Energy monitor based on the Arduino NANO
  * written by Dr Alan Collinson
  * AC Developments
  * 27th April 2020
  * Version A.4.27
  Makes use of some code from the Rotary Encoder Demo
  rot-encode-demo.ino
  written by 
  DroneBot Workshop 2019
  https://dronebotworkshop.com
*/
/* ************************************************************ */
/* *** Included header files ********************************** */
/* ************************************************************ */

// include the liquid crystal library:
#include <LiquidCrystal.h>

// include the EEPROM library:
#include <EEPROM.h>

// Include Arduino Wire library for I2C
#include <Wire.h>

// Include AC Developments Utilities library for string manipulation
#include <acd_utils.h>

// Include AC Developments Energy Monitor library
#include <acd_eMonitor.h>

 #include "acd_constants.h"
 
/* ************************************************************ */
/* *** GLOBAL VARIABLE DEFINITIONS **************************** */
/* ************************************************************ */

// Define string with response to Master

char buf[ANSWERSIZE];
char end_log[ANSWERSIZE] = "-1.000";
bool menu_flag;

byte command = CMD_0;
bool updated = false;

/* Shaft encoder clock pin state */
 bool currentStateCLK;
 bool previousStateCLK; 

/* Menu variables */ 
String Menu_header[MENU_COUNT +1]; 
int parameter[MENU_COUNT];
int element[MENU_COUNT];

/* Preset parameter variables */
int ratio[CT_COUNT] = { 1000, 1500, 2000, 2500, 3000, 3600};
int resistor[BURDEN_COUNT] = {14, 27, 33, 47, 50, 100};
int sample[SAMPLE_COUNT]= {730, 1460, 2190, 2920, 3650, 4380};
int illum[ILLUM_COUNT] =  {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
int vref[VREF_COUNT] = {44,45,46,47,48,49,50,51};
int vmains[VMAINS_COUNT] = {230, 232, 234, 236, 238, 240, 244, 246, 248, 250};
int logger[LOG_COUNT] = { LOG_OFF, LOG_ON };
int restor[RESTORE_COUNT] = {SET_FLAG, RESET_FLAG};

/* selected parameter variables */
int  menu;
int  ct; 
int  burden;
int  samp;
int  illumin; 
int  datalog; 
int  restore_defaults;
int  index;

/* Energy Monitor variables */

  double voltage_reference;
  double calib;
  double current;
  double volts;
  double power;
  

/* ************************************************************ */
/* *** INSTANCE DEFINITIONS *********************************** */
/* ************************************************************ */

/* Initialize the LCD library by allocating the NANO control and data pins */

LiquidCrystal lcd(RS, EN, D4, D5, D6, D7);

/* Initialize the ACD Utility library */

ACDUtils utils;

ACDeMonitor emoni(ADC_RES, NEUTRALPIN, INPUTPIN);

/* ************************************************************ */
/* *** FUNCTION DEFINITIONS *********************************** */
/* ************************************************************ */

void setLED(byte cmd) {

  if (cmd == CMD_0)
    digitalWrite(ledPin, LOW);
  if (cmd == CMD_1)
    digitalWrite(ledPin, HIGH);
}

/* ************************************************************ */
/* *** DISPLAY FUNCTION DEFINITIONS *************************** */
/* ************************************************************ */


/* Function that displays a page of information on the LCD screen
 *  in the form of line 1 and line 2
 */

void display_page(char *str1, char *str2) {
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(str1); 
  lcd.setCursor(0, 1);
  lcd.print(str2); 
  delay(DISPLAYTIME);
}

/* Function to display a series of pages of information, partcularly at start-up */

void display_flash(void) {

  display_page(COMPANY, MODEL);
  display_page(BY, AUTHOR);
  display_page(SERIAL_NO, VERSION);
  
}

void display_settings(void) {
  int i_count;

  for (i_count = 1; i_count < MENU_COUNT +1; i_count ++) {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(Menu_header[i_count]);
    lcd.setCursor(0,1);
    
    switch (i_count) {
      case CT_PARAM: // CT Ratio
        lcd.print(ct);
        break;
      case BURDEN_PARAM:
        lcd.print(burden);
        lcd.print(" Ohms");
        break;
      case SAMPLE_PARAM:
        lcd.print(samp);
        break;
      case ILLUM_PARAM:
        lcd.print(illumin);
        break;
      case VREF_PARAM:
        lcd.print(voltage_reference);
        break;
      case VMAINS_PARAM:
        lcd.print(volts);
        break;
      case LOG_PARAM:
        if (datalog == LOG_ON)
          lcd.print("ON ");
        else
          lcd.print("OFF");
        break;
        case RESTORE_PARAM:
        EEPROM.update(EE_RESTORE, restore_defaults);
        if (restore_defaults == SET_FLAG)
          lcd.print("DEFAULTS ARE SET");
        else {
          lcd.print("RESTORE DEFAULTS");
          set_defaults();
          set_parameters();
        }
        break;
       case MENU_COUNT:
        lcd.print(calib);
        break;      
      }
  delay(DISPLAYTIME);
    
  }
  
}

/* Function to format the display during noral operation */

void display_standard(void) {

  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("Power");
  lcd.setCursor(14,0);
  lcd.print("kW");
  lcd.setCursor(0,1);
  lcd.print("Current");
  lcd.setCursor(14,1);
  lcd.print("A");
  
}

/* ************************************************************ */
/* *** MENU FUNCTION DEFINITIONS ****************************** */
/* ************************************************************ */

/* Sub Menu Function  - Changes the value of the selected parameter */

void sub_menu(int title, int *values) {
  
  int counter;
  int old_counter;
  int i_max;
  int value;

    i_max = element[title] - 1; // number of different elements in 'title' array */
    counter = parameter[title]; // start by pointing at the current element in the parameter array
    old_counter = counter;      // used to see if a new value has been selected
    
  /* Clear the screen and display the sub-menu header */
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(Menu_header[title]);
  /* display the original value and display the currently selected value  */
    value = values[counter];
    lcd.setCursor(12,1);
    lcd.print(value);
    lcd.setCursor(0,1);
    lcd.print(value);

    while (digitalRead(input_SW))  // do while the the encoder "select" is not pressed
      {
  
       // Read the current state of inputCLK
      currentStateCLK = digitalRead(input_CLK);
    
      // If the previous and the current state of the inputCLK are different and the CLK is low then a pulse has occured
      if (currentStateCLK != previousStateCLK && !currentStateCLK) { 
       
        // If the inputDT state is different than the inputCLK state then 
        // the encoder is rotating clockwise, so increase counter
        if (digitalRead(input_DT) != currentStateCLK) { 
          counter ++;
          } else {
          // Encoder is rotating anticlockwise
          counter --;
          }
        }
      
      if (counter > i_max) {
            counter = MIN_COUNT;
          }
      else if (counter < MIN_COUNT) { 
            counter = i_max;
      }
      if (counter != old_counter) { 
      value = values[counter];
 //     Serial.print(counter);
 //     Serial.print(values[counter]);
 //     Serial.println(value);
      lcd.setCursor(0,1);
      lcd.print("        ");
      lcd.setCursor(0,1);
      lcd.print(value);
      delay(DEBOUNCE);
      old_counter = counter;
     }     
   
   // Update previousStateCLK with the current state
   previousStateCLK = currentStateCLK;  
    }
  while (!digitalRead(input_SW))
  {
      // wait for the switch to be released again 
  }
  parameter[title] = counter;
 }
 
/* Main Menu */

 void main_menu(int *cnt, int i_max) {
  
  int old_counter;
  int counter;
 
    counter = *cnt;
    old_counter = *cnt;

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("MAIN MENU");
    lcd.setCursor(0,1);
    lcd.print(Menu_header[counter]);

    while (!digitalRead(input_SW))
  {
      // wait for the switch to be released again 
      delay(DEBOUNCE);
  }

    while (digitalRead(input_SW))
      {
      // Read the current state of inputCLK
      currentStateCLK = digitalRead(input_CLK);
    
      // If the previous and the current state of the inputCLK are different and the CLK is low then a pulse has occured
      if (currentStateCLK != previousStateCLK && !currentStateCLK) { 
       
        // If the inputDT state is different than the inputCLK state then 
        // the encoder is rotating clockwise, so increase counter
        if (digitalRead(input_DT) != currentStateCLK) { 
          counter++;
          } else {
          // Encoder is rotating anticlockwise
          counter--;
          }
       }
       if (counter > i_max) {
            counter = MIN_COUNT;
          }
      else if (counter < MIN_COUNT) { 
            counter = i_max;
      }
      if (counter != old_counter) { 
      lcd.setCursor(0,1);
      lcd.print("        ");
      lcd.setCursor(0,1);
      lcd.print(Menu_header[counter]);
      delay(DEBOUNCE);
      old_counter = counter;
     }     
   
   // Update previousStateCLK with the current state
   previousStateCLK = currentStateCLK;  
    }
  while (!digitalRead(input_SW))
  {
      // wait for the switch to be released again 
  }

  switch (counter) {
    case CT_PARAM: // CT Ratio
     sub_menu(counter, ratio);
     ct = ratio[parameter[CT_PARAM]];
     break;
   case BURDEN_PARAM:
     sub_menu(counter, resistor);
     burden = resistor[parameter[BURDEN_PARAM]];
     break;
   case SAMPLE_PARAM:
     sub_menu(counter, sample);
     samp = sample[parameter[SAMPLE_PARAM]];
     break;
   case ILLUM_PARAM:
    sub_menu(counter, illum); 
    illumin = illum[parameter[ILLUM_PARAM]];
    analogWrite(PWM_OUT,map(illumin, 0, 10, MIN_BRIGHTNESS, MAX_BRIGHTNESS));
    break;
   case VREF_PARAM:
   sub_menu(counter, vref);
   voltage_reference = (float)vref[parameter[VREF_PARAM]]/10.0;
   break; 
   case VMAINS_PARAM:
   sub_menu(counter, vmains);
   volts = (float)vmains[parameter[VREF_PARAM]];
   break; 
   case LOG_PARAM:
    sub_menu(counter, logger);
    datalog = logger[parameter[LOG_PARAM]];
    break;      
   case RESTORE_PARAM:
   sub_menu(counter, restor);
   restore_defaults =restor[parameter[RESTORE_PARAM]];
   break; 
  }
  *cnt = counter;
 }

 
 void requestEvent() {

  if (updated) {
    if (datalog == LOG_OFF)
      Wire.write(end_log[index]);
    else
      Wire.write(buf[index]);
    index++;
    if (index >= ANSWERSIZE) {
      updated = false;
      index = 0;
      }
    }
  else {
    Wire.write(NUL);
  }
}

 void receiveEvent(int howMany) {
  
  command = Wire.read();  //cmd
  index = 0;
}

int debounced(int sw) {

  if (!digitalRead(sw)) {
    delay(DEBOUNCE);
    while (!digitalRead(sw)) {
      /* do nothing, just wait */
      }
   delay(DEBOUNCE);
   return 1;
    }
   else return 0;
}

 void set_parameters(void) {

   parameter[MENU_PARAM] = EEPROM.read(EE_MENU);
   parameter[CT_PARAM] = EEPROM.read(EE_CT);
   parameter[BURDEN_PARAM] = EEPROM.read(EE_BURDEN);
   parameter[SAMPLE_PARAM] = EEPROM.read(EE_SAMPLE);
   parameter[ILLUM_PARAM] = EEPROM.read(EE_ILLUM);
   parameter[VREF_PARAM] = EEPROM.read(EE_VREF);
   parameter[VMAINS_PARAM] = EEPROM.read(EE_VMAINS);  
   parameter[LOG_PARAM] = EEPROM.read(EE_LOG);
   parameter[RESTORE_PARAM] = EEPROM.read(EE_RESTORE); 
  
}
void set_defaults(void) {

    EEPROM.update(EE_MENU, INITIAL_MENU);  
    EEPROM.update(EE_CT, INITIAL_CT); 
    EEPROM.update(EE_BURDEN, INITIAL_BURDEN); 
    EEPROM.update(EE_SAMPLE, INITIAL_SAMPLE); 
    EEPROM.update(EE_ILLUM, INITIAL_ILLUM); 
    EEPROM.update(EE_VREF, INITIAL_VREF);
    EEPROM.update(EE_VMAINS, INITIAL_VMAINS);   
    EEPROM.update(EE_LOG, INITIAL_LOG); 

}

void update_eeprom(void) {
  
  EEPROM.update(EE_MENU, parameter[MENU_PARAM]);
  EEPROM.update(EE_CT, parameter[CT_PARAM]);
  EEPROM.update(EE_BURDEN, parameter[BURDEN_PARAM]);
  EEPROM.update(EE_SAMPLE, parameter[SAMPLE_PARAM]);
  EEPROM.update(EE_ILLUM, parameter[ILLUM_PARAM]);
  EEPROM.update(EE_VREF, parameter[VREF_PARAM]);
  EEPROM.update(EE_VMAINS, parameter[VMAINS_PARAM]); 
  EEPROM.update(EE_LOG, parameter[LOG_PARAM]);
  
 }


/* ************************************************************ */
/* *** SETUP FUNCTION ***************************************** */
/* ************************************************************ */

 void setup() { 

  menu_flag = true;
   
  pinMode(ledPin, OUTPUT);
  
  /* Set encoder pins as inputs  */
  pinMode(input_CLK,INPUT);
  pinMode(input_DT,INPUT);
  
  /* Set encoder switch pin as input */
  pinMode (input_SW, INPUT);

  /* Set Screen illumination pin to PWM Output */
  pinMode (PWM_OUT, OUTPUT);

  /* Initialise the LCD */
  lcd.begin(COLUMN, ROW);
  lcd.clear();
      
  /* Setup Serial Monitor */
  Serial.begin (9600);
   
  // Initialize I2C communications as Slave
  Wire.begin(SLAVE_ADDR);
  
  // Function to run when data requested from master
  Wire.onRequest(requestEvent); 
  Wire.onReceive(receiveEvent);
  
  /* Initialise the EEPROM for the first time */
   
  if (EEPROM.read(EE_RESTORE)) {
    EEPROM.write(EE_RESTORE, SET_FLAG);
    set_defaults();
  }
    
   /* Initialise the parameter index pointers */ 
   set_parameters();
    
   /* Initialise the menu array sizes  */ 
   
  element[MENU_PARAM] = MENU_COUNT;
  element[CT_PARAM] = CT_COUNT;
  element[BURDEN_PARAM] = BURDEN_COUNT;
  element[SAMPLE_PARAM] = SAMPLE_COUNT;
  element[ILLUM_PARAM] = ILLUM_COUNT;
  element[VREF_PARAM] = VREF_COUNT;
  element[VMAINS_PARAM] = VMAINS_COUNT;
  element[LOG_PARAM] = LOG_COUNT;
  element[RESTORE_PARAM] = RESTORE_COUNT;

  /* Initialise the menu headers */
    
  Menu_header[MENU_PARAM] = "RETURN          ";
  Menu_header[CT_PARAM] = "CT RATIO        ";
  Menu_header[BURDEN_PARAM] = "BURDEN          ";
  Menu_header[SAMPLE_PARAM] = "SAMPLES         ";
  Menu_header[ILLUM_PARAM] = "ILLUM           ";
  Menu_header[VREF_PARAM] = "REF VOLTAGE     ";
  Menu_header[VMAINS_PARAM] = "MAINS VOLTAGE   ";  
  Menu_header[LOG_PARAM] = "ENABLE LOG      ";
  Menu_header[RESTORE_PARAM] = "RESTORE DEFAULTS";
  Menu_header[CALIBRAT] = "CALIBRATION     ";

/* Initialise the function variables */
  
  menu = parameter[MENU_PARAM];
  ct = ratio[parameter[CT_PARAM]];
  burden = resistor[parameter[BURDEN_PARAM]];
  samp = sample[parameter[SAMPLE_PARAM]];
  illumin = illum[parameter[ILLUM_PARAM]];
  datalog = logger[parameter[LOG_PARAM]];
  voltage_reference = (float)vref[parameter[VREF_PARAM]]/10.0;
  volts = (float)vmains[parameter[VMAINS_PARAM]];
  restore_defaults = restor[parameter[RESTORE_PARAM]];
  analogWrite(PWM_OUT,map(illumin, 0, 10, MIN_BRIGHTNESS, MAX_BRIGHTNESS));
  calib = ct/burden;

  display_flash();
  display_settings();
  display_standard();
   
   // Read the initial state of inputCLK
   // Assign to previousStateCLK variable
   previousStateCLK = digitalRead(input_CLK);

   menu_flag = false;
 } 

/* ************************************************************ */
/* *** LOOP FUNCTION ****************************************** */
/* ************************************************************ */

 void loop() { 
  
  if (debounced(input_SW)) {
    menu_flag = true;
    main_menu(&menu, MENU_COUNT-1);  
    update_eeprom();
    calib = ct/burden;
    display_settings();
    display_standard();
    menu_flag = false;
    }
  else
    {
    setLED(command);  
    current = emoni.rms_calc(samp, voltage_reference, calib);
    updated = true;
    power = volts*current/1000;
    utils.ftoa(current, buf, 6, 3); 
    lcd.setCursor(8, 0);
    lcd.print(power);
    // print the current:
    lcd.setCursor(8, 1);
    lcd.print(current);
    }
  }
 
