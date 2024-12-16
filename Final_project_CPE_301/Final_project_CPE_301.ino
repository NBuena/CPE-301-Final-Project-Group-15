//By: Gali Hacohen, Noah Buenaventura
//Noah's Notes: State Controller working, attachInterrupt working, LED's working. DHT working, need to change from serial monitor output to LCD output.
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
const long intervalDHT = 60000;
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
//RTC
RTC_DS1307 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
// Stepper motor
#define stepper_revs (2038)
//LCD
const int RS = 52, EN = 53, D4 = 48, D5 = 49, D6 = 50, D7 = 51;
LiquidCrystal lcd(RS, EN, D4, D5, D6, D7);

//Used for control over states of the cooler 0 = Disabled; 1 = Idle; 2 = Running; 3 = Error
volatile unsigned int state = 0;
volatile unsigned char read;

void setup() {
  //Serial Init
  U0init(9600);
  //RTC Init
  rtc.begin();
  DateTime now = rtc.now();
  read = now.year()/1000;
  U0putchar(read);
  //On/Off Button Init
  *ddr_e &= (0xEF);
  *port_e |= (0x10);
  //LED Init
  *ddr_h |= (0x78);
  /*Stepper Motor Init
  stepper(stepper_revs, 22,24,26,28);
  stepper. setspeed(50);*/
  //LCD Init
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  
  attachInterrupt(digitalPinToInterrupt(togglePin), Toggle, FALLING);
}

void loop() {
  unsigned long currentMillis = millis();
  switch (state) {
    case 0:
      //yellow LED ON
      *port_h &= (0x00);
      *port_h |= (0x01 << 6);
      lcd.clear();
      break;
    case 1:
      //green LED ON
      *port_h &= (0x00);
      *port_h |= (0x01 << 3);
      break;
    case 2:
      //blue LED ON
      *port_h &= (0x00);
      *port_h |= (0x01 << 5);
      break;
    case 3:
      //red LED ON
      *port_h &= (0x00);
      *port_h |= (0x01 << 4);
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.write("ERROR: LOW WATER");
      lcd.setCursor(0,1);
      lcd.write("REFILL & RESET!");
      break;
  }
  if (state != 0 && state != 3) {
    if (currentMillis - millisDHT >= intervalDHT) {
      millisDHT = currentMillis;
      int chk = DHT.read11(DHTPIN);
      lcd.setCursor(0,0);
      lcd.write("Temp: ");
      lcd.print(DHT.temperature);
      lcd.write("C");
      lcd.setCursor(0,1);
      lcd.write("Humidity: ");
      lcd.print(DHT.humidity);
      lcd.write("%");
    }
    
  }
  /*
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Check if any reading failed
  if (isnan(temperature) || isnan(humidity)) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Error: Sensor");
    lcd.setCursor(0, 1);
    lcd.print("Read Failed");
    return;
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temperature);
  lcd.print("C");

  lcd.setCursor(0, 1);
  lcd.print("Humidity: ");
  lcd.print(humidity);
  lcd.print("%");

  delay(1000);
  */

  //USART EXAMPLE CODE -----
  /*
  unsigned char cs1;
  while (U0kbhit()==0){}; // wait for RDA = true
  cs1 = U0getchar();    // read character
  U0putchar(cs1);     // echo character
  */

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
  /*
   unsigned char fly;
   fly = *myUDR0;
   return fly;
  */
}

// Wait for USART0 (myUCSR0A) TBE to be set then write character to transmit buffer
void U0putchar(unsigned char U0pdata)
{
  while(!( *myUCSR0A  & (1 << 5)));
  *myUDR0  = U0pdata;
}
/*
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
  read = time.year()
}*/
//Toggles Between DISABLED state and ON.
//Pressing the button in any other state than DISABLED will set state to DISABLED
//Pressing the button in DISABLED sets state to IDLE
void Toggle () {
  state += 1;
  if (state > 1) {
    state = 0;
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
