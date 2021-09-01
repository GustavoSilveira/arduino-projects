// Exercício: PR09 - Conversão digital analógica com somador ponderado tipo malha R-2R
// Aluno: Gustavo de Souza Silveira - 31/08/2021 
// Prof. Nelson Garcia - UTFPR-CT - EL82E - Eletrônica Digital 2

// UNO  - Usar pinos 0, 1, 2, 3 para malha R-R2
// MEGA - Usar pinos 18, 19, 20, 21 para malha R-R2

const int modePin = A0;

void setup()
{
  DDRD = B11111111; //PORTA 0 A 7
  DDRB = B00111111; //PORTA 8 9 10 11 12 13
  pinMode(modePin, INPUT);
}

void loop()
{
  static byte i=0;
  PORTD = i;
  // Modo lento ou rápido
  if (digitalRead(modePin) == HIGH)
  	delay(2000);
  else
    delay(50);
  i++; 	
}
