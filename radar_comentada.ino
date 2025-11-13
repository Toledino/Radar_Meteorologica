// 1. BIBLIOTECAS UTILIZADAS

#include <Servo.h>            //  Pode ser verificada no Gerenciador de Bibliotecas, mas deve funcionar imediatamente sem instalar        
#include <Wire.h>             //  Vem com a IDE do Arduino         
#include <Adafruit_GFX.h>     //  Digite "Adafruit GFX" no Gerenciador de Bibliotecas e instale   
#include <Adafruit_SSD1306.h> //  Digite "Adafruit SSD1306" no Gerenciador de Bibliotecas e instale

// 2. DEFINIÇÕES DE HARDWARE
// Configuração do Display OLED 128x64

#define SCREEN_WIDTH 128  // Define uma constante chamada SCREEN_WIDTH com o valor 128 (número de pixels na horizontal)
#define SCREEN_HEIGHT 64  // Define uma constante chamada SCREEN_HEIGHT com o valor 64 (número de pixels na vertical)
#define OLED_RESET -1     // O valor -1 diz à biblioteca para NÃO usar um pino de reset do Arduino

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define TRIG_PIN 2     // Pino digital para o Sensor Ultra Som Triger
#define ECHO_PIN 3    // Pino digital para o Sensor Ultra Som Echo
#define SERVO_PIN 5   // Pino digital para controlar o Servo
#define BUZZER_PIN 6  // Pino digital para controlar o Buzzer

// 3. VARIÁVEIS GLOBAIS

Servo servo;  // Cria um objeto da classe Servo para interagir com o motor servo.

const int maxDistance = 50;  // Distância máxima (em cm) que o radar tentará medir e exibir
const int centerX = SCREEN_WIDTH / 2;  // Calcula o centro horizontal (X) do display (64) é o meio de 128
const int centerY = SCREEN_HEIGHT - 1;  // Calcula o centro vertical (Y) do display, ajustado para a base inferior (63). 64 - 1 = 63 
const int maxRadius = 50;  // Define o raio máximo do radar na tela (em pixels)

// Define uma estrutura (tipo de dado) chamada Blip para armazenar as informações de um objeto detectado
// Dentro desta estrutura, temos as variáveis: angulo,distancia,timestamp e active
struct Blip {
 float angle;  // Armazena o ângulo em que o objeto foi detectado
 int distance; // Armazena a distância do objeto em cm
 unsigned long timestamp;  // Armazena o tempo em milissegundos em que o objeto foi detectado
 // Será melhor explicado na funcao drawBlips()
 bool active;  // Flag booleana que indica se o blip ainda deve ser exibido
};

Blip blips[20];                // Cria um array que pode armazenar até 20 objetos (blips) simultaneamente
int blipCount = 0;             // Contador que rastreia quantos blips estão ativos no array
unsigned long lastUpdate = 0;  // Armazena o tempo da última atualização do display. Usado para controlar a taxa de quadros (FPS)
const long fadeTime = 3000;    // Tempo (em milissegundos) que um blip leva para "desaparecer" (3 segundos)
int currentAngle = 0;          // Armazena o ângulo atual do servo motor (para exibição na tela)
int currentDistance = 0;       // Armazena a última distância medida (para exibição na tela) 
unsigned long lastToggle = 0;  // Armazena o tempo do último acionamento do buzzer
bool toggleState = false;      // Armazena o estado (ligado/desligado) do buzzer

// 4. FUNÇÕES
//  Início da função que mede a distância usando o sensor ultrassônico
float measureDistance() {

 digitalWrite(TRIG_PIN, LOW);    // Garante que o pino TRIG esteja baixo
 delayMicroseconds(2);           // Pequena pausa 2 microsegundos
 digitalWrite(TRIG_PIN, HIGH);   // Envia um pulso de 10 microssegundos para o TRIG para iniciar a medição
 delayMicroseconds(10);          // Pequena pausa 10 microsegundos
 digitalWrite(TRIG_PIN, LOW);    // Termina o pulso

// Mede a duração do pulso de retorno no pino ECHO. Espera até 30ms (30000µs) por segurança
 long duration = pulseIn(ECHO_PIN, HIGH, 30000);
// Se o sensor enviar o pulso e não receber um eco de volta dentro desses 30.000 microssegundos, a função pulseIn() retorna o valor 0
 if (duration == 0) return maxDistance;
// Cálculo da distância: (duração do pulso * velocidade do som em cm/µs) / 2 (ida e volta)
  float distance = duration * 0.034 / 2;
// Retorna a distância. Se for maior que maxDistance, retorna maxDistance
  return (distance > maxDistance) ? maxDistance : distance;
}

// Início da função que adiciona um novo objeto detectado (blip) ao array
void addBlip(float angle, int distance) {
// Verifica se há espaço no array (máx. 20) e se a distância é menor que a máxima permitida
 if (blipCount < 20 && distance < maxDistance) {

  blips[blipCount].angle = angle;         // Armazena o ângulo atua
  blips[blipCount].distance = distance;   // Armazena a distância medida
  blips[blipCount].timestamp = millis();  // Armazena o tempo atual em milissegundos (o momento da detecção)
  blips[blipCount].active = true;         // Marca o blip como ativo

  blipCount++;                            // Incrementa o contador de blips ativos
 }
}
// Início da função que verifica e remove blips que já "desapareceram" (expiraram)
void updateBlips() {

 unsigned long currentTime = millis();    // Armazena o tempo atual
  for (int i = 0; i < blipCount; i++) {   // Inicia o loop para percorrer todos os blips ativos
// Verifica se o blip está ativo E se o tempo decorrido desde a detecção é maior que o tempo de fade (3 segundos) 
   if (blips[i].active && (currentTime - blips[i].timestamp > fadeTime)) {
      blips[i].active = false;  // Se expirou, marca o blip como inativo
// Inicia um loop para mover todos os blips seguintes para uma posição anterior (compactando o array para remover o blip inativo)
  for (int j = i; j < blipCount - 1; j++) {  //  Move o blip para a posição anterior
      blips[j] = blips[j + 1];  //  Move o blip para a posição anterior
   }                            //  Fim do loop de compactação
   blipCount--;                 //  Decrementa o contador de blips ativos
   i--;                         //  Decrementa o contador principal do loop
   }                            //  Fim do bloco if de verificação de tempo
  }                             //  Fim do loop principal
 }                              //  Fim da função
//  Início da função que desenha os blips ativos no display
void drawBlips() {

 unsigned long currentTime = millis();   // Armazena o tempo atual
  for (int i = 0; i < blipCount; i++) {  // Inicia o loop para percorrer todos os blips ativos
  if (blips[i].active) {                 // Verifica se o blip está marcado como ativo
//  Calcula a "idade" do blip como uma fração de 0.0 a 1.0, onde 1.0 é totalmente desaparecido
    float age = (currentTime - blips[i].timestamp) / (float)fadeTime;
//  Cálculo da Coordenada X: Transforma a distância (cm) e o ângulo (graus) do blip em coordenadas X para a tela, usando trigonometria
    int x = centerX + (blips[i].distance * maxRadius / maxDistance) * cos(radians(blips[i].angle));
//  Cálculo da Coordenada Y: Transforma a distância e o ângulo em coordenadas Y para a tela. Subtrai de centerY porque no display o Y=0 é o topo
    int y = centerY - (blips[i].distance * maxRadius / maxDistance) * sin(radians(blips[i].angle));
//  Verifica se o blip é "jovem" (idade menor que 0.5, ou seja, nos primeiros 1.5 segundos)
  if (age < 0.5) {
//  Se for jovem, desenha um pequeno X (ou cruz) para destacá-lo (parecendo uma detecção recente)
/*
A função drawLine() é fornecida pela biblioteca Adafruit GFX e tem a seguinte sintaxe:
display.drawLine(x0, y0, x1, y1, cor);
x0, y0: Coordenadas (X, Y) do ponto inicial da linha.
x1, y1: Coordenadas (X, Y) do ponto final da linha.
cor: A cor da linha (neste caso,liga os pixel do display)

A Primeira Linha (Diagonal Principal)

display.drawLine(x-2, y-2, x+2, y+2, SSD1306_WHITE);
Ponto central do blip: O x e y são as coordenadas (em pixels) do local exato do objeto, calculadas a partir da distância 
e do ângulo usando trigonometria (linhas 109 e 111).
Ponto Inicial (x0, y0): x-2,y-2.Este ponto está 2 pixels à esquerda e 2 pixels acima do centro (x, y).
Ponto Final (x1, y1): x+2,y+2. Este ponto está 2 pixels à direita e 2 pixels abaixo do centro (x, y).
Resultado: Esta linha desenha a diagonal descendente (de cima à esquerda para baixo à direita) da cruz

A Segunda Linha (Diagonal Secundária)

display.drawLine(x-2, y+2, x+2, y-2, SSD1306_WHITE);

Ponto Inicial (x0, y0): x-2,y+2.Este ponto está 2 pixels à esquerda e 2 pixels abaixo do centro (x, y).
Ponto Final (x1, y1): x+2,x-2.Este ponto está 2 pixels à direita e 2 pixels acima do centro (x, y).
Resultado: Esta linha desenha a diagonal ascendente (de baixo à esquerda para cima à direita) da cruz.

 */
  display.drawLine(x-2, y-2, x+2, y+2, SSD1306_WHITE);
  display.drawLine(x-2, y+2, x+2, y-2, SSD1306_WHITE);

 } else {  //  Se o blip for mais "velho" (idade > 0.5)
 
 display.drawPixel(x, y, SSD1306_WHITE); // Desenha apenas um único pixel
   }                                     // Fim do bloco if de idade
  }                                      // Fim do bloco if de blip ativo
 }                                       // Fim do loop
}                                        // Fim da função


void drawRadar() {     // Início da função que desenha as linhas de referência do radar
//  Desenha um círculo no centro, com metade do raio máximo, como referência de distância
display.drawCircle(centerX, centerY, maxRadius / 2, SSD1306_WHITE);
//  Desenha um círculo no centro, com o raio máximo
display.drawCircle(centerX, centerY, maxRadius, SSD1306_WHITE);
}
//  Início da função que exibe as informações de ângulo e distância na tela
void drawInfo() {

 display.setTextSize(1);                  // Define o tamanho da fonte
 display.setTextColor(SSD1306_WHITE);     // Significa ligar o Pixel
 display.setCursor(0, 0);                 // Move o cursor para (x=0,y=0) no display
 display.print("Angulo:");                // Escreve o rótulo "Angulo" no display (0,0)
 display.print(currentAngle);             // Imprime o valor atual do ângulo do servo
 display.setCursor(72, 0);                // Move o cursor para (x=70,y=0) no display
 display.print("Dist:");                  // Escreve o rótulo "Dist" no display (72,0)
 display.print(currentDistance);          // Imprime a última distância medida
 display.print("cm");                     // Imprime a unidade de medida
}
//  Início da função para controlar o buzzer com base na distância
void handleBuzzer() {

 unsigned long currentTime = millis();  // Armazena o tempo atual
  if (currentDistance < 25) {           // Verifica se um objeto está mais perto que 25 cm
//  Se a distância for menor que 25 cm, verifica se passaram 500ms (0.5 segundo) desde o último toque
  if (currentTime - lastToggle >= 500) {
  toggleState = !toggleState;           // Inverte o estado do buzzer (liga se estava desligado, desliga se estava ligado)
  digitalWrite(BUZZER_PIN, toggleState);// Aciona o buzzer com o novo estado
  lastToggle = currentTime;             // Atualiza o tempo do último acionamento do buzzer
 }                                      // Fim do if de tempo
 } else {                               // Se a distância for maior ou igual a 25 cm

 digitalWrite(BUZZER_PIN, LOW);          //  Desliga o buzzer
 toggleState = false;                    //  Reseta o estado do buzzer
  }                                      //  Fim do if-else
 }                                       //  Fim da função
// 5. Função SETUP
// Função de Configuração - Executada uma vez
void setup() {

 pinMode(TRIG_PIN, OUTPUT);            //  Configura o pino TRIG (9) como SAÍDA
 pinMode(ECHO_PIN, INPUT);             //  Configura o pino ECHO (10) como ENTRADA
 pinMode(BUZZER_PIN, OUTPUT);          //  Configura o pino BUZZER (12) como SAÍDA
 digitalWrite(BUZZER_PIN, LOW);        //  Garante que o buzzer comece desligado
 servo.attach(SERVO_PIN);              //  Associa o objeto servo ao pino 11
 servo.write(0);                       //  Move o servo para a posição inicial (0 graus)

//  Inicializa o display OLED com a configuração de energia e o endereço I2C (0x3C, que é o mais comum)
 if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
//  Se a inicialização falhar (o display não for detectado), o programa trava neste loop infinito
 for(;;);
 }
  display.clearDisplay();                 // Limpa a tela         
  display.setTextColor(WHITE);             // Significa ligar o Pixel
  // Primeira tela
  display.setTextSize(1);                // Define o tamanho da fonte
  display.setCursor(22, 3);              // Move o cursor para (x=22,y=3) no display
  display.println("Radar Ultrassom! ");  // Escreve o rótulo "Radar Ultrassom! " no display (22,3)
  display.setTextSize(1);                // Define o tamanho da fonte
  display.setCursor(25, 20);             // Move o cursor para (x=25,y=20) no display
  display.println("Toledino");           // Escreve o rótulo "Toledino" no display (25,20)
  display.display();                     // Atualiza o Display. Torna o texto visível na tela.
  delay(1200);                           // O codigo fica esperando 1200 ms
  display.clearDisplay();                // Limpa a tela 

  // Segunda tela (Créditos)
  display.setTextSize(1.5);              // Define o tamanho da fonte
  display.setCursor(20, 20);             // Move o cursor para (x=20,y=22) no display
  display.println("by Guto ");           // Escreve o rótulo "by Guto " no display (20,22)
  display.display();                     // Atualiza o Display. Torna o texto visível na tela.
  delay(1000);                           // O codigo fica esperando 1000 ms
  display.clearDisplay();                // Limpa a tela 
}
// 6. Função LOOP
// Função Principal - Executada em loop
void loop() {
static int angle = 0;                      // Variável que guarda o ângulo atual do servo (inicializa em 0 na primeira vez)
static bool forward = true;                // Variável que indica a direção do movimento do servo (true = aumentando ângulo, false = diminuindo)
static unsigned long lastServoMove = 0;    // Guarda o tempo da última movimentação do servo
static unsigned long lastMeasurement = 0;  // Guarda o tempo da última medição do sensor
unsigned long currentTime = millis();      // Armazena o tempo atual do sistema
// Verifica se passaram 25 milissegundos desde a última movimentação (controla a velocidade de varredura)
if (currentTime - lastServoMove >= 25) {   
servo.write(angle);                        // Move o servo para o angle atual
currentAngle = angle;                      // Atualiza a variável global do ângulo (para exibição)

if (forward) {                             // Se a direção for para frente (0 para 180 graus)
angle += 2;                                // Aumenta o ângulo em 2 graus
if (angle >= 180) {                        // Se o ângulo atingir 180 graus, inverte a direção
angle = 180;                               // declara o angulo 180
forward = false;                           // Define a direção para trás
 }
} else {                                   // Se a direção for para trás (180 para 0 graus)
angle -= 2;                                // Diminui o ângulo em 2 graus.
if (angle <= 0) {                          // Se o ângulo atingir 0 graus, inverte a direção
angle = 0;                                 // Declara o angulo 0
forward = true;                            // Define a direção para frente
   }
  }
 lastServoMove = currentTime;              // Atualiza o tempo da última movimentação do servo
}
//  Controle do Sensor: Verifica se passaram 100 milissegundos desde a última medição
if (currentTime - lastMeasurement >= 100) {    
 currentDistance = (int)measureDistance(); // Realiza a medição e armazena o valor (convertido para inteiro)
  if (currentDistance < maxDistance) {     // Se a distância estiver dentro do limite máximo  
  addBlip(angle, currentDistance);         // Chama a função para adicionar a detecção como um novo blip  
 }
 lastMeasurement = currentTime;            // Atualiza o tempo da última medição
}                                          // Fim do bloco de controle do sensor.

handleBuzzer();                            // Chama a função para verificar e controlar o buzzer

if (currentTime - lastUpdate >= 33) {      // Verifica se passaram 33ms (aproximadamente 30 FPS) desde a última atualização da tela
display.clearDisplay();                    // Limpa o conteúdo anterior do display
updateBlips();                             // Chama a função para remover blips expirados

drawRadar();                               // Desenha os círculos de referência do radar
/*
Esta é a parte mais essencial da programação de um radar, pois utiliza trigonometria básica para transformar
um ângulo e um raio (distância) em coordenadas X e Y na tela.
A função principal aqui é calcular a ponta da "linha de varredura" (o braço do radar) para que ela siga a posição atual do motor servo

1. O Conceito Geométrico (Trigonometria)
Para encontrar qualquer ponto (X, Y) em um círculo, dado um ângulo (grau) e um raio (R),
usamos as seguintes fórmulas trigonométricas, com o centro do círculo sendo a origem (0, 0):
X = R*cos(angulo) 
Y = R*sin(angulo) Onde:
R é o raio (neste caso,maxRadius, o comprimento máximo da linha de varredura).
grau é o ângulo (neste caso, angle, a posição atual do servo)
cos(cosseno) e sin(seno) são as funções trigonométricas.

2. A Adaptação ao Código (Transformando Coordenadas)
O código precisa adaptar a fórmula trigonométrica a três fatores importantes:

A. Centralização
O centro do radar não é a origem (0, 0) do display. O centro do display é (centerX,centerY}).
Para mover o resultado da trigonometria para o local correto na tela, basta somar as coordenadas do centro:
sweepX = centerX + X 
sweepY = centerY + Y

B. Inversão do Eixo Y
Em matemática, o eixo Y positivo aponta para cima.Em displays (como o OLED), o eixo Y positivo aponta para baixo (o Y=0 está no topo).
Para que o radar se comporte de forma intuitiva (ângulo 0 à direita,90 para cima),precisamos inverter o resultado do seno (a coordenada Y).
Portanto, a fórmula para Y fica com uma subtração em vez de uma soma:         
sweepY = centerY - R*sin(angulo)

As funções trigonométricas da biblioteca padrão do Arduino (cos() e sin()) esperam que o ângulo seja fornecido em radianos, não em graus.
Como o motor servo trabalha com ângulos em graus (angle vai de 0 a 180), o código utiliza a função auxiliar radians(angle) para fazer a conversão.
*/

// Cálculo da Coordenada X: 
// 1. Pega o centro horizontal (centerX) 
// 2. Adiciona o comprimento do raio (maxRadius)
// 3. Multiplicado pelo cosseno do ângulo, que foi convertido de graus para radianos (radians(angle))
int sweepX = centerX + maxRadius * cos(radians(angle));
//  1.Pega o centro vertical (centerY). 
//  2.Subtrai o resultado (para inverter o eixo Y). 
//  3.O resultado é o comprimento do raio (maxRadius) multiplicado pelo seno do ângulo convertido.
int sweepY = centerY - maxRadius * sin(radians(angle));
//  Esta linha conecta o Centro Fixo do radar (centerX, centerY) com a Ponta Variável (sweepX, sweepY), 
//  criando a ilusão de um braço de radar que gira junto com o motor servo
display.drawLine(centerX, centerY, sweepX, sweepY, SSD1306_WHITE);
drawBlips();          //  Desenha todos os blips ativos
drawInfo();           //  Desenha as informações de Ângulo e Distância
display.display();    //  Envia o buffer de dados para o display para que as alterações sejam visíveis
lastUpdate = currentTime;   //  Atualiza o tempo da última atualização do display
 }                           //  Fim do bloco de controle do display
}                           //  Fim do loop()Fim do loop()

