By: Gali Hacohen, Noah Buenaventura

#include <DHT.h>
#include <LiquidCrystal_I2C.h>

//POINTERS

#define DHTPIN 2        
#define DHTTYPE DHT22
      
// Initialize the DHT sensor
DHT dht(DHTPIN, DHTTYPE);

LiquidCrystal_I2C lcd(0x27, 16, 2);


// LCD STUFF 
void setup() {
  dht.begin();
  

  lcd.begin();
  lcd.backlight();
  lcd.setCursor(0, 0);

}

void loop() {
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
}
