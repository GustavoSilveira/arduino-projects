// Exercício: PR08 - Registrador de deslocamento 74HC595 e display
// Aluno: Gustavo de Souza Silveira - 16/08/2021 
// Prof. Nelson Garcia - UTFPR-CT - EL82E - Eletrônica Digital 2

const int dataPin = 8;
const int latchPin = 9;
const int clockPin = 10;
const int potPin = A0;

// Mapa dos segmentos de numeros de 0 a 9
const byte SEGMENT_MAP[] = {63, 6, 91, 79, 102, 109, 125, 7, 127, 111, 128, 255};

void setup() {
  pinMode(dataPin, OUTPUT);  
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);  
  pinMode(potPin, INPUT);    
}

void loop() {  
  static int i = 0;
  int timeMs;

  // Leitura do potenciometro para definir atraso de contagem de 100ms a 1000ms
  timeMs = map(analogRead(potPin), 0, 1023, 100, 1000); 
  
  // Atualiza e mantém o pino latch em LOW enquanto ocorre a transmissão
  digitalWrite(latchPin, LOW); 
  // Transmite o valor de i%10 ao 74HC595, a começar pelo bit mais significativo
  shiftOut(dataPin, clockPin, MSBFIRST, SEGMENT_MAP[i%10]); 
  // Modifica o pino latch para HIGH para sinalizar o 74HC595
  digitalWrite(latchPin, HIGH); 
    
  i++;
  delay(timeMs);
}
