//By: Gali Hacohen, Noah Buenaventura
//Noah's Notes: State controller is next, then attachinterrupt for the On/Off button. Also handle GPIO for LED's

#include <DHT.h>
#include <LiquidCrystal.h>

//POINTERS

volatile unsigned char *myUCSR0A = (unsigned char *)0x00C0;
volatile unsigned char *myUCSR0B = (unsigned char *)0x00C1;
volatile unsigned char *myUCSR0C = (unsigned char *)0x00C2;
volatile unsigned int  *myUBRR0  = (unsigned int *) 0x00C4;
volatile unsigned char *myUDR0   = (unsigned char *)0x00C6;
#define RDA 0x80
#define TBE 0x20  
#define DHTPIN 2        
#define DHTTYPE DHT22
      
// Initialize the DHT sensor
DHT dht(DHTPIN, DHTTYPE);

LiquidCrystal lcd(0x27, 16, 2);

void setup() {
  U0init(9600);
  dht.begin();
  
  lcd.begin();
  lcd.backlight();
  lcd.setCursor(0, 0);
  //attach.interrupt();
}

void loop() {
  //Used for control over states of the cooler 0 = Disabled; 1 = Idle; 2 = Running; 3 = Error
  volatile int state = 0;
 /* switch (state) [
    case "0":

  ]*/

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
  //USART EXAMPLE CODE -----
  /*
  unsigned char cs1;
  while (U0kbhit()==0){}; // wait for RDA = true
  cs1 = U0getchar();    // read character
  U0putchar(cs1);     // echo character
  */
}
//
// function to initialize USART0 to "int" Baud, 8 data bits,
// no parity, and one stop bit. Assume FCPU = 16MHz.
//
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
//
// Read USART0 RDA status bit and return non-zero true if set
//
unsigned char U0kbhit()
{
  if(*myUCSR0A & (1<<7)){
    return 1;
  }else {
    return 0;
  }
}
//
// Read input character from USART0 input buffer
//
unsigned char U0getchar()
{
  return *myUDR0;
  /*
   unsigned char fly;
   fly = *myUDR0;
   return fly;
  */
}
//
// Wait for USART0 (myUCSR0A) TBE to be set then write character to
// transmit buffer
//
void U0putchar(unsigned char U0pdata)
{
  while(!( *myUCSR0A  & (1 << 5)));
  *myUDR0  = U0pdata;
}