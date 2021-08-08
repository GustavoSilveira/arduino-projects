// Exercício: Prova de Suficiência 2020/2
// Aluno: Gustavo de Souza Silveira - 23/10/2020
// Prof. Sergio Moribe - UTFPR-CT

#include <LiquidCrystal.h>

//Define os pinos do Display LCD no Arduino
const int rs = 8, en = 9, d4 = 10, d5 = 11, d6 = 12, d7 = 13;

//Instancia objeto lcd
LiquidCrystal lcd(rs, en, d4, d5, d6, d7); //lcd em 4 bits

// quantidade de linhas e colunas do LCD
const int linhasLCD = 2, colunasLCD = 16; 

// texto inicial do editor de texto
const String textoInicial = "Proj. PWM";

// linha vazia para limpar o LCD
const String linhaVazia   = "                ";

// Configuração das interrupções
#define pinIniciar 2
#define pinParar 3

// Declaração da função de estado
void estadoCircuito();
void botaoLigar();
void botaoDesligar();

// Estado do PWM
bool estado = false; 

// Saidas PWM
#define pinLed1 5
#define pinLed2 6

// millis
unsigned long time = 0;
unsigned long timeAnterior= 0;

void setup() {
  
  // Configurações do PWM que aciona os LEDs
  pinMode(pinLed1, OUTPUT); //Onde vai sair o sinal PWM 1
  pinMode(pinLed2, OUTPUT); //Onde vai sair o sinal PWM 2
  
  // Pinos de interrupção
  pinMode(pinIniciar, INPUT_PULLUP);  
  pinMode(pinParar, INPUT_PULLUP); 
  
  // Configuração do LCD
  lcd.begin(colunasLCD, linhasLCD);      
  lcd.setCursor(0,0); //Posiciona cursor na 1a linha  
  lcd.print(textoInicial); 
  lcd.setCursor(0,1); //Posiciona cursor na 2a linha 
  lcd.noBlink(); //Pisca o cursor   
  
  // Modo padrão: Desligado
  estadoCircuito();
  
  // Instancia das interrupções
  attachInterrupt(digitalPinToInterrupt(pinIniciar), botaoLigar, FALLING);
 
  attachInterrupt(digitalPinToInterrupt(pinParar), botaoDesligar, FALLING);    
}

void loop() {
  char LCD[16];
  
  int canal01 = 0;
  int canal02 = 0;   
  
  time = millis(); // tempo atual
    
  if (estado) {

    // Lendo valores analogicos    
    canal01 = analogRead(A0);
    canal02 = analogRead(A1);

    // percentual de cada canal
    int percentualCanal01 = map(canal01 , 0, 1023, 0, 100);
    int percentualCanal02 = map(canal02 , 0, 1023, 0, 100);

    percentualCanal01 = percentualCanal01 > 0 ? percentualCanal01 : 0;
    percentualCanal02 = percentualCanal02 > 0 ? percentualCanal02 : 0;

    if ( (time - timeAnterior) > 100 ) {
      // Canal 01    
      dtostrf(percentualCanal01,3,0,LCD);

      lcd.setCursor(0,1); //Posiciona cursor na 2a linha  
      lcd.print(linhaVazia);
      lcd.setCursor(2,1); //Posiciona cursor na 2a linha   
      lcd.print("  ");   
      lcd.setCursor(2,1); //Posiciona cursor na 2a linha   
      lcd.print(LCD); 
      lcd.print("%");

      // Canal 02    
      dtostrf(percentualCanal02,3,0,LCD);

      lcd.setCursor(8,1); //Posiciona cursor na 2a linha   
      lcd.print("  ");     
      lcd.setCursor(8,1); //Posiciona cursor na 2a linha   
      lcd.print(LCD);
      lcd.print("%");
      
      timeAnterior = time;
    }
    
  }
  
  // Atualizando LEDs
  analogWrite(pinLed1, canal01 / 4);
  analogWrite(pinLed2, canal02 / 4);

}

void estadoCircuito() {	
  
    lcd.setCursor(0,1); //Posiciona cursor na 2a linha 
	lcd.print(linhaVazia);
  	lcd.setCursor(0,1);
  
    if (estado) { 
    	lcd.print("Ligado...             ");
        delay(5000);   	      
    } else {
        lcd.print("Desligado...          ");
    }
    
}

void botaoLigar() {
	estado = true;  
  	estadoCircuito();
}

void botaoDesligar() {
  	estado = false;
  	estadoCircuito();    
}
    
