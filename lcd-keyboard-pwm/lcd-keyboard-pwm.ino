// Exercício: Projeto LCD Teclado e PWM
// Aluno: Gustavo de Souza Silveira - 27/09/2020
// Prof. Sergio Moribe - UTFPR-CT

#include <LiquidCrystal.h>

//Define os pinos do Display LCD no Arduino
const int rs = 8, en = 9, d4 = 10, d5 = 11, d6 = 12, d7 = 13;

//Instancia objeto lcd
LiquidCrystal lcd(rs, en, d4, d5, d6, d7); //lcd em 4 bits

// quantidade de linhas e colunas do LCD
const int linhasLCD = 2, colunasLCD = 16; 

// texto inicial do editor de texto
const String textoInicial = "Proj. LCD e PWM";

// linha vazia para limpar o LCD
const String linhaVazia   = "                ";

void setup() {
  pinMode(3, OUTPUT); //Onde vai sair o sinal PWM 1
  pinMode(5, OUTPUT); //Onde vai sair o sinal PWM 2
  
  lcd.begin(colunasLCD, linhasLCD);      
  lcd.setCursor(0,0); //Posiciona cursor na 1a linha  
  lcd.print(textoInicial); 
  lcd.setCursor(0,1); //Posiciona cursor na 2a linha 
  lcd.blink(); //Pisca o cursor 
  delay(4000);  
  lcd.noBlink(); 
  lcd.setCursor(0,0); //Posiciona cursor na 1a linha  
  lcd.print(linhaVazia);   
}

void loop() {
  char LCD[16];
  
  // escrevendo valores no osciloscópio
  int canal01 = analogRead(A0);
  int canal02 = analogRead(A1);
  
  analogWrite(3, canal01 / 4);
  analogWrite(5, canal02 / 4);
  
  // percentual de cada canal
  int percentualCanal01 = map(canal01 , 0, 1023, 0, 100);
  int percentualCanal02 = map(canal02 , 0, 1023, 0, 100);
  
  percentualCanal01 = percentualCanal01 > 0 ? percentualCanal01 : 0;
  percentualCanal02 = percentualCanal02 > 0 ? percentualCanal02 : 0;
  
  // Canal 01
  lcd.setCursor(0,0); //Posiciona cursor na 1a linha   
  dtostrf(percentualCanal01,3,0,LCD);
  lcd.print("% Canal 01: ");  
  lcd.print(LCD);  
  lcd.print("  ");  
  
  // Canal 02
  lcd.setCursor(0,1); //Posiciona cursor na 2a linha   
  dtostrf(percentualCanal02,3,0,LCD);
  lcd.print("% Canal 02: ");  
  lcd.print(LCD);  
  lcd.print("  ");   

  
  //delay(100); //100ms
}
