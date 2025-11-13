// BIBLIOTECAS

#include <SPI.h>      
#include <Wire.h>
#include <Adafruit_GFX.h>     
#include <Adafruit_SSD1306.h> 
#include <Adafruit_Sensor.h>  
#include <DHT.h>              

#define SCREEN_WIDTH 128  
#define SCREEN_HEIGHT 64  
#define OLED_RESET -1  

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define sensor A0       
#define DHTPIN 2        
#define DHTTYPE DHT11   
#define BUZZER_PIN 5    

// VARIÁVEIS GLOBAIS

DHT dht(DHTPIN, DHTTYPE); 
int gasLevel = 0;         
String quality = "";      

//  FUNÇÕES

void air_sensor() { 
  gasLevel = analogRead(sensor); 


  if (gasLevel < 400) {  
    quality = " Boa!"; 
  } else if (gasLevel >= 400 && gasLevel < 450) {  
    quality = " Ruim!";  
  } else if (gasLevel >= 450 && gasLevel < 500) {  
    quality = "Muito Ruim!";  
  } else if (gasLevel >= 500 && gasLevel < 700) {  
    quality = "Toxico!";  
  } else {  
    quality = " Toxico"; 
  }

  if (gasLevel >= 500) {
    digitalWrite(BUZZER_PIN, HIGH); 
  } else {
    digitalWrite(BUZZER_PIN, LOW); 
  }

  display.setTextColor(WHITE);  
  display.setTextSize(1);       
  display.setFont();            
  display.setCursor(1, 5);      
  display.println("Qualidade do Ar:"); 
  display.setCursor(5, 23); 
  display.println(gasLevel); 
  display.setCursor(45, 23); 
  display.println(quality); 
}

void sendSensor() {      
  
  float h = dht.readHumidity();      
  float t = dht.readTemperature();   

  display.setTextColor(WHITE);  
  display.setTextSize(1);      
  display.setFont();  
  display.setCursor(0, 43); 
  display.println("Tempertura:");
  display.setCursor(80, 43); 
  display.println(t);  
  display.setCursor(114, 43); 
  display.println("C");  
  display.setCursor(0, 56);  
  display.println(" Umidade  :");  
  display.setCursor(80, 56);  
  display.println(h);  
  display.setCursor(114, 56); 
  display.println("%");  
} 

void setup() {
  pinMode(sensor, INPUT);  
  dht.begin(); 
  pinMode(BUZZER_PIN, OUTPUT); 
 
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
  for(;;);
 }

  display.clearDisplay(); 
  display.setTextColor(WHITE); 
  display.setTextSize(1);  
  display.setCursor(25, 0);
  display.println("Metereologia"); 
  display.setTextSize(1);
  display.setCursor(25, 20);
  display.println("Toledino");
  display.display();
  delay(1200);
  display.clearDisplay();
  display.setTextSize(1.5);
  display.setCursor(20, 20); 
  display.println("by Guto "); 
  display.display();
  delay(1000); 
  display.clearDisplay();
}

void loop() {
  display.clearDisplay(); 
  air_sensor();  
  sendSensor(); 
  display.display();
  delay(10);
}
