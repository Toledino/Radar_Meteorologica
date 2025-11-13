// BIBLIOTECAS UTILIZADAS

#include <Servo.h>            
#include <Wire.h>             
#include <Adafruit_GFX.h>     
#include <Adafruit_SSD1306.h> 

#define SCREEN_WIDTH 128  
#define SCREEN_HEIGHT 64  
#define OLED_RESET -1     

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define TRIG_PIN 2     
#define ECHO_PIN 3    
#define SERVO_PIN 5   
#define BUZZER_PIN 6  

// VARIÁVEIS GLOBAIS

Servo servo; 

const int maxDistance = 50; 
const int centerX = SCREEN_WIDTH / 2;  
const int centerY = SCREEN_HEIGHT - 1;  
const int maxRadius = 50; 

struct Blip {
 float angle;  
 int distance; 
 unsigned long timestamp;  
 bool active; 
};

Blip blips[20];                
int blipCount = 0;             
unsigned long lastUpdate = 0;  
const long fadeTime = 3000;    
int currentAngle = 0;          
int currentDistance = 0;       
unsigned long lastToggle = 0;  
bool toggleState = false;      

float measureDistance() {

 digitalWrite(TRIG_PIN, LOW);    
 delayMicroseconds(2);           
 digitalWrite(TRIG_PIN, HIGH);   
 delayMicroseconds(10);          
 digitalWrite(TRIG_PIN, LOW);    

 long duration = pulseIn(ECHO_PIN, HIGH, 30000);

 if (duration == 0) return maxDistance;
  float distance = duration * 0.034 / 2;
  return (distance > maxDistance) ? maxDistance : distance;
}

void addBlip(float angle, int distance) {
 if (blipCount < 20 && distance < maxDistance) {

  blips[blipCount].angle = angle;         
  blips[blipCount].distance = distance;   
  blips[blipCount].timestamp = millis();  
  blips[blipCount].active = true;         
  blipCount++;                           
 }
}

void updateBlips() {

 unsigned long currentTime = millis();   
  for (int i = 0; i < blipCount; i++) {  
   if (blips[i].active && (currentTime - blips[i].timestamp > fadeTime)) {
      blips[i].active = false;  
  for (int j = i; j < blipCount - 1; j++) {  
      blips[j] = blips[j + 1]; 
   }                           
   blipCount--;                
   i--;                        
   }                           
  }                            
 }                              

void drawBlips() {
 unsigned long currentTime = millis();   
  for (int i = 0; i < blipCount; i++) {  
  if (blips[i].active) {                 
    float age = (currentTime - blips[i].timestamp) / (float)fadeTime;
    int x = centerX + (blips[i].distance * maxRadius / maxDistance) * cos(radians(blips[i].angle));
    int y = centerY - (blips[i].distance * maxRadius / maxDistance) * sin(radians(blips[i].angle));
  if (age < 0.5) {

 display.drawLine(x-2, y-2, x+2, y+2, SSD1306_WHITE);
 display.drawLine(x-2, y+2, x+2, y-2, SSD1306_WHITE);
 display.drawLine(x-2, y-2, x+2, y+2, SSD1306_WHITE);
 display.drawLine(x-2, y+2, x+2, y-2, SSD1306_WHITE);

 } else {  
 display.drawPixel(x, y, SSD1306_WHITE); 
   }                                     
  }                                      
 }                                       
}                                       

void drawRadar() {     
 display.drawCircle(centerX, centerY, maxRadius / 2, SSD1306_WHITE);
 display.drawCircle(centerX, centerY, maxRadius, SSD1306_WHITE);
}

void drawInfo() {
 display.setTextSize(1);                  
 display.setTextColor(SSD1306_WHITE);     
 display.setCursor(0, 0);                 
 display.print("Angulo:");                
 display.print(currentAngle);            
 display.setCursor(72, 0);                
 display.print("Dist:");                  
 display.print(currentDistance);          
 display.print("cm");                     
}

void handleBuzzer() {

 unsigned long currentTime = millis(); 
  if (currentDistance < 25) {           
  if (currentTime - lastToggle >= 500) {
  toggleState = !toggleState;           
  digitalWrite(BUZZER_PIN, toggleState);
  lastToggle = currentTime;             
  }                                      
 } else {                               

 digitalWrite(BUZZER_PIN, LOW);         
 toggleState = false;                    
  }                                      
 }                                       

void setup() {

 pinMode(TRIG_PIN, OUTPUT);            
 pinMode(ECHO_PIN, INPUT);             
 pinMode(BUZZER_PIN, OUTPUT);          
 digitalWrite(BUZZER_PIN, LOW);        
 servo.attach(SERVO_PIN);              
 servo.write(0);                       

 if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
  for(;;);
 }
  display.clearDisplay();                       
  display.setTextColor(WHITE);             
  display.setTextSize(1);                
  display.setCursor(22, 3);              
  display.println("Radar Ultrassom! ");  
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
static int angle = 0;                    
static bool forward = true;                
static unsigned long lastServoMove = 0;   
static unsigned long lastMeasurement = 0;  
unsigned long currentTime = millis();     

if (currentTime - lastServoMove >= 25) {   
servo.write(angle);                        
currentAngle = angle;                      

if (forward) {                             
angle += 2;                                
if (angle >= 180) {                        
angle = 180;                               
forward = false;                           
 }
} else {                                   
angle -= 2;                                
if (angle <= 0) {                          
angle = 0;                                
forward = true;                            
   }
  }
 lastServoMove = currentTime;              
}

if (currentTime - lastMeasurement >= 100) {    
 currentDistance = (int)measureDistance(); 
  if (currentDistance < maxDistance) {     
  addBlip(angle, currentDistance);          
 }
 lastMeasurement = currentTime;            
}                                          

handleBuzzer();                            

if (currentTime - lastUpdate >= 33) {      
display.clearDisplay();                    
updateBlips();                             
drawRadar();                               

int sweepX = centerX + maxRadius * cos(radians(angle));
int sweepY = centerY - maxRadius * sin(radians(angle));

display.drawLine(centerX, centerY, sweepX, sweepY, SSD1306_WHITE);
drawBlips();         
drawInfo();          
display.display();    
lastUpdate = currentTime;   
 }                          
}                         

