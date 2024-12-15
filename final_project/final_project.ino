By: Gali Hacohen, Noah Buenaventura

#include <DHT.h>
#include <LiquidCrystal_I2C.h>
#include <stepper.h>
#include <ezLED.h>

//POINTERS

// LED's
#define LED_yellow pin 9
#define LED_blue pin 8
#define LED_red pin 7
#define LED_green pin 6

// Stepper motor
#define stepper_revs (2038)

// Temprature
#define DHTPIN 2        
#define DHTTYPE DHT22

// Initialize the LED's
void steup(){
  pinMode(LED_yellow, OUTPUT);
  pinMode(LED_blue, OUTPUT);
  pinMode(LED_red, OUTPUT);
  pinMode(LED_green, OUTPUT);


}

// LED's stuff
void loop()(
  
)

// Initialize the Stepper Motor
  stepper(stepper_revs, 22,24,26,28);

// Initialize the DHT sensor
DHT dht(DHTPIN, DHTTYPE);

LiquidCrystal_I2C lcd(0x27, 16, 2);

//Stepper Motor stuff
  void steup(){
    stepper. setspeed(50);

  }

void loop(){

  stepper.step(stepper_revs);
  delay(500);


  stepper.step(-stepper_revs);
  delay(500);
}
// LCD stuff 
void setup() {
  dht.begin();
  

  lcd.begin();
  lcd.backlight();
  lcd.setCursor(0, 0);

}

void loop() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

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

  delay(500);
}
