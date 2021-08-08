// Exercício: PR02 - Pisca-Pisca com Arduino
// Aluno: Gustavo de Souza Silveira - 23/06/2021 
// Prof. Nelson Garcia - UTFPR-CT - EL82E - Eletrônica Digital 2

// Define pinos dos LEDs
const byte led[]  = {2, 3, 4, 5, 6, 7, 8, 9}; 
const int totalLeds = sizeof(led)/sizeof(led[0]);

// Millis
unsigned long time = 0;
unsigned long timeAnterior = 0;
const long intervaloPiscaMilis = 300;

// Ordem do Pisca-Pisca
int ordem = -1;
int posicaoLed = 0;
int sinalLedAmarelo = 1;

void setup()
{    
  // Configurando saídas digitais
  for(int i = 0; i < totalLeds; i++){
    pinMode(led[i], OUTPUT);
    digitalWrite(led[i], LOW);
  }    
}

void loop()
{
  time = millis(); // tempo atual 
  
  // Condicional do intervalo
  if ( (time - timeAnterior) > intervaloPiscaMilis ) {
    
    // Segmento de 7 Leds
    digitalWrite(led[posicaoLed-ordem], LOW);
    digitalWrite(led[posicaoLed], HIGH);   
    
    // Led Amarelo
    digitalWrite(led[totalLeds - 1], ((sinalLedAmarelo == 1) ? HIGH : LOW));
	
    // Condicional e variáveis de controle
    if (  ( posicaoLed == totalLeds - 2 ) || ( posicaoLed == 0 ) ) {
      ordem = ordem * -1;
    }
    
    sinalLedAmarelo = sinalLedAmarelo * -1;
      
    posicaoLed = posicaoLed + ordem;
    
    timeAnterior = time;
  }
}
