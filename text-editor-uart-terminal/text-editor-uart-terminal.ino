// Exercício: Editor de texto e Interface Serial Terminal UART
// Aluno: Gustavo de Souza Silveira - 21/09/2020 - Atualizado
// Prof. Sergio Moribe - UTFPR-CT

#include <LiquidCrystal.h>
#include <Keypad.h>

//Define os pinos do Display LCD no Arduino
const int rs = 14, en = 15, d4 = 16, d5 = 17, d6 = 18, d7 = 19;

//Instancia objeto lcd
LiquidCrystal lcd(rs, en, d4, d5, d6, d7); //lcd em 4 bits

//Define pinos das Linhas e Colunas do Teclado 4x4
byte pLinhas[]  = {9,8,7,6}; 
byte pColunas[] = {5,4,3,2};
//Define mapa de caracteres que corresponde cada tecla
char teclas[4][4] = {{'1',' 2','3','A'},
                     {'4','5','6','B'},
                     {'7','8','9','C'},
                     {'*','0','#','D'}};

//Instancia objeto teclado
Keypad teclado = Keypad( makeKeymap(teclas), pLinhas, pColunas, 4, 4);

// quantidade de linhas e colunas do LCD
const int linhasLCD = 2, colunasLCD = 16; 

// teclas digitadas em memoria
String textoDigitado = ""; 

// texto inicial do editor de texto
const String textoInicial = "Editor + UART:  ";

// linha vazia para limpar o LCD
const String linhaVazia   = "                ";

// Textos definidos
String textos[4] = { " Projeto de Ensino UTFPR", 
                      " Exemplo de terminal Serial", 
                      " Sistemas de Telecomunicacoes",
                      " Departamento Academico de Eletronica"
                    };

// Dados digitados pelo monitor serial
char buffer[16];

void quebraLinha()
{  
  lcd.setCursor(0,0); //Posiciona cursor na 1a linha   
  lcd.print(linhaVazia); // limpa 1a linha 
  lcd.setCursor(0,0); //Posiciona cursor na 1a linha  
  lcd.print(textoDigitado); // copiando conteudo para primeira linha
  lcd.setCursor(0,1); //Posiciona cursor na 2a linha
  lcd.print(linhaVazia);    
  lcd.setCursor(0,1); //Posiciona cursor na 2a linha
  textoDigitado = "";  
  Serial.println(""); // Quebra linha no monitor serial
}

void reiniciaEditor()
{
  // Comandos do LCD
  lcd.setCursor(0,0); //Posiciona cursor na 1a linha 
  lcd.print(textoInicial); //Escreve string no LCD      
  //Limpa 2a linha e reposiciona no inicio dela
  lcd.setCursor(0,1); //Posiciona cursor na 2a linha
  lcd.print(linhaVazia);
  lcd.setCursor(0,1); //Posiciona cursor na 2a linha  
  
  // Comandos do monitor serial
  for(int i = 0; i < 100; i++){
  Serial.println("");
  }
  Serial.println(textoInicial);  
}

void setup()
{   
  Serial.begin(2400);
  lcd.begin(colunasLCD, linhasLCD);  
  reiniciaEditor();
  delay(2000);
  lcd.setCursor(0,0); //Posiciona cursor na 1a linha  
  lcd.print("* Quebra linha  "); 
  lcd.setCursor(0,1); //Posiciona cursor na 2a linha
  lcd.print("# Limpa editor  ");    
  delay(4000);
  reiniciaEditor();  
  lcd.blink(); //Pisca o cursor    
}
 
void loop()
{
  //Verifica se alguma tecla foi pressionada
  char tecla_pressionada = teclado.getKey();
  
  Serial.setTimeout(10);
  int len = Serial.readBytes(buffer,16);
 
  if(len > 0) {
    lcd.write(buffer);  //Escreve a leitura do monitor no LCD
    Serial.print(buffer);
    textoDigitado = textoDigitado + buffer;     
  }
  
  if (tecla_pressionada || len){ //Se tem tecla apertada
    if (isDigit(tecla_pressionada)){  
      lcd.write(tecla_pressionada);  //Escreve Tecla no LCD
      Serial.print(tecla_pressionada); // Escreve Tecla no monitor serial
      textoDigitado = textoDigitado + tecla_pressionada;       
    }
    
    // Teclas programadas
    if(tecla_pressionada == '#'){ 
		reiniciaEditor(); // reinicia o editor de texto
    }else if(tecla_pressionada == 'A'){      
      Serial.println(textos[0]);
      quebraLinha();
    }else if(tecla_pressionada == 'B'){      
      Serial.println(textos[1]);
      quebraLinha();
    }else if(tecla_pressionada == 'C'){
      Serial.println(textos[2]);     
      quebraLinha();
    }else if(tecla_pressionada == 'D'){      
      Serial.println(textos[3]);      
      quebraLinha();
    }         
    
    // quebra a linha ao fim ou pressionando *
    if( (textoDigitado.length() >= colunasLCD) || (tecla_pressionada == '*')) { 
      quebraLinha();
    }    
  }  
  delay(10);
}
 
