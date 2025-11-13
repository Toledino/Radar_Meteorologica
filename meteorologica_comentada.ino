// 1. BIBLIOTECAS UTILIZADAS

#include <SPI.h>      
#include <Wire.h>
#include <Adafruit_GFX.h>      // Digite "Adafruit GFX" no gerenciador de bibliotecas e instale 
#include <Adafruit_SSD1306.h>  // Digite Adafruit SSD1306 no gerenciador de bibliotecas e instale 
#include <Adafruit_Sensor.h>   // Digite "Adafruit Unified Sensor" no gerenciador de bibliotecas e instale
#include <DHT.h>               // Digite "DHT sensor library" no gerenciador de bibliotecas e instale

// 2. DEFINIÇÕES DE HARDWARE
// Configuração do Display OLED 128x64 

#define SCREEN_WIDTH 128  // Define uma constante chamada SCREEN_WIDTH com o valor 128 (número de pixels na horizontal).
#define SCREEN_HEIGHT 64  // Define uma constante chamada SCREEN_HEIGHT com o valor 64 (número de pixels na vertical).
#define OLED_RESET -1  //  O valor -1 diz à biblioteca para NÃO usar um pino de reset do Arduino

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Pinos dos Sensores (Sem alteração)
#define sensor A0       // Pino analógico para o Sensor de Gás (MQ-X)
#define DHTPIN 2        // Pino digital para o Sensor DHT
#define DHTTYPE DHT11   // Define o modelo do sensor (Pode ser DHT22 ou DHT11)
#define BUZZER_PIN 5    // Pino digital para acionar o Buzzer (D5)

// 3. VARIÁVEIS GLOBAIS

DHT dht(DHTPIN, DHTTYPE); // Esta linha cria o objeto dht, usando as constantes definidas DHTPIN  e DHTTYPE
int gasLevel = 0;         // Declara uma variável inteira (int) chamada gasLevel e a inicializa com o valor 0
String quality = "";      // Declara uma variável do tipo String (texto) chamada quality e a inicializa como vazia ("")

// 4. FUNÇÕES

// Declara uma função chamada air_sensor que não retorna nenhum valor (void) e não aceita argumentos. 
// Seu propósito é ler, classificar e exibir dados do sensor de gás.

void air_sensor() {   // Função para ler o sensor de gás e classificar a qualidade do ar
  gasLevel = analogRead(sensor); // Chama a função analogRead() para ler o valor no pino A0). 
  // O valor lido é um número inteiro entre 0 (0 Volts) e 1023 (5 Volts) e é armazenado na variável gasLevel

  // Lógica de classificação da qualidade do ar
  if (gasLevel < 400) {  // Primeira Condição. Se o gasLevel lido for menor que 400
    quality = " Boa!";  // A variável global quality recebe o texto " Boa!"
  } else if (gasLevel >= 400 && gasLevel < 450) {  //  Segunda Condição. Se o nível de gás estiver entre 400 e 449.
    quality = " Ruim!";  // A variável global quality recebe o texto " Ruim!"
  } else if (gasLevel >= 450 && gasLevel < 500) {  //  Terceira Condição. Se o nível de gás estiver entre 450 e 499.
    quality = "Muito Ruim!";  // A variável global quality recebe o texto "Muito Ruim!"
  } else if (gasLevel >= 500 && gasLevel < 700) {  // Quarta Condição.Se o nível de gás estiver entre 500 e 699
    quality = "Toxico!";  //  A qualidade é classificada como "Toxico!"
  } else {  // Condição Final (Pior Qualidade).Se nenhuma das condições acima for verdadeira ou o nível for maior que 700
    quality = " Toxico"; //  A qualidade é classificada como "Toxico!"
  }

  if (gasLevel >= 500) {
    digitalWrite(BUZZER_PIN, HIGH); // Liga o buzzer (alerta sonoro)
  } else {
    digitalWrite(BUZZER_PIN, LOW);  // Desliga o buzzer
  }

  // Configurações de exibição da Qualidade do Ar
  display.setTextColor(WHITE);  // significa ligar o Pixel
  display.setTextSize(1);       // define o tamanho da fonte
  display.setFont();            //  define para usar a fonte padrão  da biblioteca GFX.
  display.setCursor(1, 5);      // Move o cursor para (x=1, y=5) no display
  display.println("Qualidade do Ar:"); // Escreve o rótulo "Qualidade do Ar" no display (1,4)
  
  // Exibição do Nível de Gás e Qualidade
  display.setCursor(5, 23); // Move o cursor para (x=5, y=23) no display
  display.println(gasLevel); // Escreve o valor da variável "gasLevel" no display (5,23)

  display.setCursor(45, 23); // Move o cursor para (x=45, y=23) no display
  display.println(quality); // Escreve o valor da variável "quality" no display (45,23)
}

void sendSensor() {     // Função para ler o sensor DHT e exibir Temperatura/Umidade 
  
  float h = dht.readHumidity();      // Lê a Umidade e coloca na variável h
  float t = dht.readTemperature();   // Lê a Temperatura e coloca na variável t

  // Configurações de exibição de Temperatura e Umidade no display oled
  display.setTextColor(WHITE);  // significa ligar o Pixel 
  display.setTextSize(1);       // define o tamanho da fonte
  display.setFont();  //  define para usar a fonte padrão  da biblioteca GFX.

  // Linha da Temperatura
  display.setCursor(0, 43); // Move o cursor para (x=0, y=43) no display. O ponto (0, 0) é o canto superior esquerdo
  display.println("Tempertura:");  // Escreve o rótulo "Temperatura:" no display (0,43)
  display.setCursor(80, 43); // Move o cursor para (x=80, y=43) no display
  display.println(t);  // Escreve o rótulo "t" no display (80,43)
  display.setCursor(114, 43); // Move o cursor para (x=114, y=43) no display
  display.println("C");  // Escreve o rótulo "C" no display (114,43)

  // Linha da Umidade Relativa (RH)
  display.setCursor(0, 56);  // Move o cursor para (x=0, y=56) no display
  display.println(" Umidade  :");  // Escreve o rótulo "Umidade" no display (0,56)
  display.setCursor(80, 56);  // Move o cursor para (x=80, y=56) no display
  display.println(h);  // Escreve o rótulo "h" no display (80,56)
  display.setCursor(114, 56);  // Move o cursor para (x=114, y=56) no display
  display.println("%");  // Escreve o rótulo "%" no display (114,56)
} 

// 5. Função SETUP
// Função de Configuração - Executada uma vez
void setup() {
  pinMode(sensor, INPUT);  // define o pino do sensor A0 como de entrada
  dht.begin();  // inicializa a biblioteca do DHT11
  pinMode(BUZZER_PIN, OUTPUT); // define o pino do BUZZER como de saida
  // ------------------------------------
  // TELA DE BOAS VINDAS
  // ------------------------------------
  // função de inicialização do hardware do displlay oled via comunicação I2C
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {

 for(;;);
 }

  display.clearDisplay();  // limpa o display oled
  display.setTextColor(WHITE);  // significa ligar o Pixel 

  // Primeira tela
  display.setTextSize(1);   // define o tamanho da fonte
  display.setCursor(25, 0);  // Move o cursor para (x=25, y=0) no display
  display.println("Metereologia");  // Escreve o rótulo "Metereologia" no display (25,0) 
  display.setTextSize(1);   // define o tamanho da fonte
  display.setCursor(25, 20);   // Move o cursor para (x=25, y=20) no display
  display.println("Toledino"); // Escreve o rótulo "Toledino" no display (25,20)
  display.display(); // Atualiza o Display. Torna o texto visível na tela.
  delay(1200); // o codigo fica esperando 1200 ms 
  display.clearDisplay(); // limpa o display oled

  // Segunda tela (Créditos)
  display.setTextSize(1.5);  // define o tamanho da fonte
  display.setCursor(20, 20); // Move o cursor para (x=20, y=20) no display
  display.println("by Guto "); // Escreve o rótulo "by Guto" no display (20,20) 
  display.display();  // Atualiza o Display. Torna o texto visível na tela.
  delay(1000); // o codigo fica esperando 1000 ms 
  display.clearDisplay(); // limpa o display oled
}
// 6. Função LOOP
// Função Principal - Executada em loop
void loop() {
  display.clearDisplay(); // limpa o display oled
  air_sensor();   // Chama a Função do Gás. Executa a função air_sensor()
  sendSensor();   // Chama a Função DHT. Executa a função sendSensor()
  display.display();  // Atualiza o Display. Torna o texto visível na tela.
  delay(10); // o codigo fica esperando 10 ms
}
