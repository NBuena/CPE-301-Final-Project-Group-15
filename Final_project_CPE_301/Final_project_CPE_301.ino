//By: Gali Hacohen, Noah Buenaventura
//Noah's Notes: Water Level Sensor Working. Need to implement Fan Motor, Vent Motor, and RTC logging
//Having trouble writing using RTC to write to serial monitor w/o using serial library
#include <RTClib.h>
#include <dht.h>
#include <LiquidCrystal.h>
#include <Stepper.h>

//POINTERS
//USART pointers
volatile unsigned char *myUCSR0A = (unsigned char *)0x00C0;
volatile unsigned char *myUCSR0B = (unsigned char *)0x00C1;
volatile unsigned char *myUCSR0C = (unsigned char *)0x00C2;
volatile unsigned int  *myUBRR0  = (unsigned int *) 0x00C4;
volatile unsigned char *myUDR0   = (unsigned char *)0x00C6;
#define RDA 0x80
#define TBE 0x20 
//DHT sensor
#define DHTPIN 10       
dht DHT;
unsigned long millisDHT = 0;
unsigned long millisEr = 0;
unsigned long millisWat = 0;
const long intervalDHT = 60000;
const long intervalWat = 1000;
/*LED pin #'s
LED_yellow pin 9 [PH6]
LED_blue pin 8 [PH5]
LED_red pin 7 [PH4]
LED_green pin 6 [PH3]
*/
volatile unsigned char* ddr_h = (unsigned char*) 0x101;
volatile unsigned char* port_h = (unsigned char*) 0x102;
//On/Off Button
volatile unsigned char* ddr_e = (unsigned char*) 0x2D;
volatile unsigned char* port_e = (unsigned char*) 0x2E;
const byte togglePin = 2;
const byte resetPin = 3;
//RTC
RTC_DS1307 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
// Stepper motor
const int stepper_revs = 2038;
Stepper daStepper = Stepper(stepper_revs, 42, 44, 43, 45);
volatile unsigned char* ddr_g = (unsigned char*) 0x33;
volatile unsigned char* port_g = (unsigned char*) 0x34;
volatile unsigned char* pin_g = (unsigned char*) 0x32;
//LCD
const int RS = 52, EN = 53, D4 = 48, D5 = 49, D6 = 50, D7 = 51;
LiquidCrystal lcd(RS, EN, D4, D5, D6, D7);
//Water Sensor
volatile unsigned char* my_ADMUX = (unsigned char*) 0x7C;
volatile unsigned char* my_ADCSRB = (unsigned char*) 0x7B;
volatile unsigned char* my_ADCSRA = (unsigned char*) 0x7A;
volatile unsigned int* my_ADC_DATA = (unsigned int*) 0x78;
volatile unsigned char* ddr_l = (unsigned char*) 0x10A;
volatile unsigned char* port_l = (unsigned char*) 0x10B;
const int water_sensor_channel = 15;
const int waterThreshold = 100;
//Used for control over states of the cooler 0 = Disabled; 1 = Idle; 2 = Running; 3 = Error
volatile unsigned int state = 0;
volatile unsigned int logState = 1;

void setup() {
  //Serial Init
  U0init(9600);
  //Analog Init
  adc_init(water_sensor_channel);
  *ddr_l |= (0x08);
  //RTC Init
  rtc.begin();
  //On/Off Button Init
  *ddr_e &= (0xEF);
  *port_e |= (0x10);
  //Reset Button Init
  *ddr_e &= (0xDF);
  *port_e |= (0x20);
  //LED Init
  *ddr_h |= (0x78);
  //DC Motor Init
  *ddr_h |= (0x01);
  //Stepper Motor Init
  daStepper.setSpeed(10);
  *ddr_g &= (0xFC);
  *port_g &= (0xFC);
  //LCD Init
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  //attach Interrupts
  attachInterrupt(digitalPinToInterrupt(togglePin), Toggle, FALLING);
  attachInterrupt(digitalPinToInterrupt(resetPin), Reset, FALLING);
}

void loop() {
  unsigned long currentMillis = millis();
  switch (state) {
    case 0:
      //yellow LED ON
      *port_h &= (0x00);
      *port_h |= (0x01 << 6);
      if (state != logState) {
        DateTime now = rtc.now();
        RTCLog(now, state);
        logState = state;
      }
      lcd.clear();
      millisDHT = 0;
      millisEr = 0;
      millisWat = 0;
      break;
    case 1:
      //green LED ON
      *port_h &= (0x00);
      *port_h |= (0x01 << 3);
      if (state != logState) {
        DateTime now = rtc.now();
        RTCLog(now, state);
        logState = state;
      }
      break;
    case 2:
      //blue LED ON
      *port_h &= (0x00);
      *port_h |= (0x01 << 5);
      *port_h |= (0x01);
      if (state != logState) {
        DateTime now = rtc.now();
        RTCLog(now, state);
        logState = state;
      }
      if (DHT.temperature <= 27) {
      state = 1;
      }
      break;
    case 3:
      //red LED ON
      *port_h &= (0x00);
      *port_h |= (0x01 << 4);
      if (state != logState) {
        DateTime now = rtc.now();
        RTCLog(now, state);
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.write("ERROR: LOW WATER");
        lcd.setCursor(0,1);
        lcd.write("REFILL & RESET!");
        logState = state;
      }
      break;
  }
  if (state != 3) {
    if (*pin_g &= (0x01)) {
      daStepper.step(5);
    } else if (*pin_g &= (0x02)) {
      daStepper.step(-5);
    }
    if (state != 0) {
    if ( millisDHT == 0 || currentMillis - millisDHT >= intervalDHT) {
      millisDHT = currentMillis;
      int chk = DHT.read11(DHTPIN);
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.write("Temp: ");
      lcd.print(DHT.temperature);
      lcd.write("C");
      lcd.setCursor(0,1);
      lcd.write("Humidity: ");
      lcd.print(DHT.humidity);
      lcd.write("%");
      if (DHT.temperature > 27 && state !=2) {
      state = 2;
      }
    }
      if (water_sensor_read(water_sensor_channel) < waterThreshold) {
        state = 3;
        return 0;
      }
    }
  }
}

// function to initialize USART0 to "int" Baud, 8 data bits,
// no parity, and one stop bit. Assume FCPU = 16MHz.
void U0init(unsigned long U0baud)
{
 unsigned long FCPU = 16000000;
 unsigned int tbaud;
 tbaud = (FCPU / 16 / U0baud - 1);
 // Same as (FCPU / (16 * U0baud)) - 1;
 *myUCSR0A = 0x20;
 *myUCSR0B = 0x18;
 *myUCSR0C = 0x06;
 *myUBRR0  = tbaud;
}

// Read USART0 RDA status bit and return non-zero true if set
unsigned char U0kbhit()
{
  if(*myUCSR0A & (1<<7)){
    return 1;
  }else {
    return 0;
  }
}

// Read input character from USART0 input buffer
unsigned char U0getchar()
{
  return *myUDR0;
}

// Wait for USART0 (myUCSR0A) TBE to be set then write character to transmit buffer
void U0putchar(unsigned char U0pdata)
{
  while((*myUCSR0A & TBE)==0);
  *myUDR0  = U0pdata;
}
void adc_init(unsigned char sensor_channel)
{
  // setup the A register
  *my_ADCSRA |= 0b10000000; // set bit   7 to 1 to enable the ADC
  *my_ADCSRA &= 0b11011111; // clear bit 6 to 0 to disable the ADC trigger mode
  *my_ADCSRA &= 0b11110111; // clear bit 5 to 0 to disable the ADC interrupt
  *my_ADCSRA &= 0b11111000; // clear bit 0-2 to 0 to set prescaler selection to slow reading
  // setup the B register
  *my_ADCSRB &= 0b11110111; // clear bit 3 to 0 to reset the channel and gain bits
  *my_ADCSRB &= 0b11111000; // clear bit 2-0 to 0 to set free running mode
  // setup the MUX Register
  *my_ADMUX  &= 0b01111111; // clear bit 7 to 0 for AVCC analog reference
  *my_ADMUX  |= 0b01000000; // set bit   6 to 1 for AVCC analog reference
  *my_ADMUX  &= 0b11011111; // clear bit 5 to 0 for right adjust result
  *my_ADMUX  &= 0b11100000; // clear bit 4-0 to 0 to reset the channel and gain bits
  // set the channel number
  if(sensor_channel > 7)
  {
    // set the channel selection bits, but remove the most significant bit (bit 3)
    sensor_channel -= 8;
    // set MUX bit 5
    *my_ADCSRB |= 0b00001000;
  }
  // set the channel selection bits
  *my_ADMUX  += sensor_channel;
}
unsigned int water_sensor_read(unsigned char sensor_channel)
{
  //set VCC pin to high
  *port_l |= (0x08);
  delay(10);
  // set bit 6 of ADCSRA to 1 to start a conversion
  *my_ADCSRA |= 0x40;
  // wait for the conversion to complete
  while((*my_ADCSRA & 0x40) != 0);
  // return the result in the ADC data register
  return *my_ADC_DATA;
  //set VCC pin to low
  *port_l &= ~(0x08);
}

void RTCLog(DateTime time, volatile unsigned int transition) {
  U0putchar('S');
  U0putchar('y');
  U0putchar('s');
  U0putchar('t');
  U0putchar('e');
  U0putchar('m');
  U0putchar(' ');
  U0putchar('c');
  U0putchar('h');
  U0putchar('a');
  U0putchar('n');
  U0putchar('g');
  U0putchar('e');
  U0putchar('d');
  U0putchar(' ');
  U0putchar('t');
  U0putchar('o');
  U0putchar(' ');
  switch(transition) {
    case 0:
      U0putchar('D');
      U0putchar('I');
      U0putchar('S');
      U0putchar('A');
      U0putchar('B');
      U0putchar('L');
      U0putchar('E');
      U0putchar('D');
      break;
    case 1:
      U0putchar('I');
      U0putchar('D');
      U0putchar('L');
      U0putchar('E');
      break;
    case 2:
      U0putchar('R');
      U0putchar('U');
      U0putchar('N');
      U0putchar('N');
      U0putchar('I');
      U0putchar('N');
      U0putchar('G');
      break;
    case 3:
      U0putchar('E');
      U0putchar('R');
      U0putchar('R');
      U0putchar('O');
      U0putchar('R');
      break;
  }
  U0putchar(' ');
  U0putchar('a');
  U0putchar('t');
  U0putchar(' ');
  Serial.print(time.hour(), DEC);
  Serial.print(':');
  Serial.print(time.minute(), DEC);
  Serial.print(':');
  Serial.print(time.second(), DEC);
  Serial.print(", ");
  Serial.print(daysOfTheWeek[time.dayOfTheWeek()]);
  Serial.print(", ");
  Serial.print(time.month(), DEC);
  Serial.print('/');
  Serial.print(time.day(), DEC);
  Serial.print('/');
  Serial.print(time.year(), DEC);
  Serial.print('\n');
}
//Toggles Between DISABLED state and ON.
//Pressing the button in any other state than DISABLED will set state to DISABLED
//Pressing the button in DISABLED sets state to IDLE
void Toggle () {
  state += 1;
  if (state > 1) {
    state = 0;
  }
}
//Resets system to IDLE if in ERROR and water level high
void Reset () {
  if (state == 3 && water_sensor_read(water_sensor_channel) >= waterThreshold) {
  state = 1;
  millisDHT = 0;
  }
}
//TESTING
//LED'S: paste into loop func:
/*
  *port_h &= (0x00);
  *port_h |= (0x01 << 5);
  delay(500);
  *port_h &= (0x00);
  *port_h |= (0x01 << 4);
  delay(500);
  *port_h &= (0x00);
  *port_h |= (0x01 << 3);
  delay(500);
  *port_h &= (0x00);
  *port_h |= (0x01 << 6);
  delay(500);
*/
//STATE MACHINE
/*
  testing state machine
  state += 1;
  if(state > 3) {
  state = 0; 
  }
*/
