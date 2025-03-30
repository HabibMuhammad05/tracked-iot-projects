#include <DHT.h>

#define DHTPIN 4     
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

float temp;
float hum;

void getTempHum(){  
  temp = dht.readTemperature();
  hum = dht.readHumidity();
  delay(10);
}
