// Exercício: Recursos de oscilador, portas PWM, portas lógicas, contador binario e dado eletrônico
// Aluno: Gustavo de Souza Silveira - 01/08/2021
// Prof. Nelson Garcia - UTFPR-CT

#include <LiquidCrystal_I2C.h>


// Quantidade de linhas e colunas do display LCD
#define LCD_LINHAS 4
#define LCD_COLUNAS 20

LiquidCrystal_I2C lcd(0x27,LCD_COLUNAS,LCD_LINHAS);  // Endereço padrão 0x27 para o display 20 x 4 --> Para display real
//LiquidCrystal_I2C lcd(0x20,LCD_COLUNAS,LCD_LINHAS);  // Endereço padrão 0x20 para o display 20 x 4 --> Para display simulado

// Caractere customizado para o LCD
byte Quadrado[8] =
{
0b00000,
0b11111,
0b11111,
0b11111,
0b11111,
0b11111,
0b11111,
0b11111
};


const byte PORTA_TECLADO = A0;

// Portas de saída
const byte PORTAS[]  = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11}; 
const int PORTAS_TOTAL = sizeof(PORTAS)/sizeof(PORTAS[0]);

// Valor do resistor de cada tecla pressionada na ordem: 1, 2, 3, 4 e *, o valor pertence ao intervalo de 0 a 1023.
const int POSICAO_FUNCAO_TECLADO[]  = {126, 159, 217, 341, 786}; 

#define MARGEM_ERRO_TECLADO 10 // Para compensar a variação do valor da leitura
#define OFF_SET_TECLADO 237

int botaoAtualPressionado = 0;
int valorEntradaTecladoAnterior = 0;

// Função atual da máquina de estados
enum funcaoEstados {
  ES_OCIOSO ,
  ES_OSCILADOR ,
  ES_SAIDA_PWM ,
  ES_CHAVES_LOGICAS ,
  ES_CONTADOR_BINARIO ,
  ES_DADO_ELETRONICO 
};

// Modo da porta de saída
enum modoPortaSaida {
  ST_NENHUM ,
  ST_LOGICO ,
  ST_PWM , 
  ST_OSCILADOR
};
// Estrutura do menu
#define MENU_ITENS 28

struct MenuItens {
  int idNivelRaiz;
  int idNivelOrigem;
  int idNivelDestino;  
  int idOrdem;         // sequencia 1,2,3 e 4
  char *nome;
  int parametroOpcao; // tempo ms, contagem e outros
} menuItens[MENU_ITENS];

// Estrutura das portas de saída
struct PortasSaidas {
  byte estado; // 0: ativo ou 1: inativo
  int valorAtual;  // 0 a 1023
  byte modo; // modo logico ou PWM
  unsigned long millisIntervalo; // millis do parametro do menu
  unsigned long millisAtual;     // millis atual
} portasSaidas[PORTAS_TOTAL];

// Millis
unsigned long timeAnterior = 0;
unsigned long timeAtualDebouce = 0;
unsigned long timeAnteriorContador = 0;  
unsigned long intervaloMilisDisplay = 1000;
unsigned long intervaloMilisContador = 0;  
unsigned long intervaloMilisContadorBase = 500; // tempo base para substrair o parametro
const int timeDebouceAtraso = 50;
const int timeDebouceAtrasoRepeticao = 500;

enum funcaoEstados atualEstadoMaquina;
enum modoPortaSaida atualModoPortaSaida;

int idMenuNivelRaizAtual = 0;

int atualizarLCD = true;

int portaPosicao = 0;
int valorParametroMenu;

bool contadorAtivo = false;
int posicaoContadorAtual = 0;

int numeroDadoEletronicoAtual = 0;

//------ SETUP ------
void setup() {  
  
  //Serial.begin(9600);

  atualEstadoMaquina = ES_OCIOSO;

  // Iniciando display
  lcd.init();                      
  lcd.backlight(); // lcd.noBacklight();
  lcd.noBlink();
  lcd.noAutoscroll();
  lcd.createChar(0, Quadrado);

  configurarEntradaSaida();
  configurarMenuItems();   
  atualizarDisplay();

  randomSeed(analogRead(0) + analogRead(1));
}

//------ LOOP ------
void loop() {

  lerTeclado();

  if ( (millis() - timeAnterior) > intervaloMilisDisplay || botaoAtualPressionado > 0) {
    /*
    Serial.print(" atualEstMaq = ");
    Serial.print(atualEstadoMaquina);  
    Serial.print(" btnAtualPress = ");
    Serial.print(botaoAtualPressionado);
    Serial.print(" idMenuRaizAtual = ");
    Serial.print(idMenuNivelRaizAtual);  
    Serial.print(" portaPosicao = ");
    Serial.print(portaPosicao);      
    Serial.print(" valorParametroMenu = ");
    Serial.print(valorParametroMenu);      

    Serial.print(" posicaoContadorAtual = ");
    Serial.print(posicaoContadorAtual);    

    Serial.print(" numeroDadoEletronicoAtual = ");
    Serial.print(numeroDadoEletronicoAtual);    

    Serial.print(" contadorAtivo = ");
    Serial.println(contadorAtivo);              
    */

    atualizarLCD = botaoAtualPressionado > 0;
    timeAnterior = millis();         
  }    
  
  if ( atualizarLCD ) atualizarPosicaoMenu();
  
  atualizaValorPorta();
  
  if ( atualizarLCD ) atualizarDisplay();    

  botaoAtualPressionado = 0;
}

void lerTeclado(){

  int valorEntradaTeclado, i = 0, valorEntradaMinimo, valorEntradaMaximo;  
  unsigned long timeDebouce;
  const int totalFuncoes = sizeof(POSICAO_FUNCAO_TECLADO)/sizeof(POSICAO_FUNCAO_TECLADO[0]);

  valorEntradaTeclado = abs(analogRead(PORTA_TECLADO)-OFF_SET_TECLADO); // leitura do teclado resistivo

  if ( valorEntradaTeclado <= 0 ) return;  

  valorEntradaMinimo = (valorEntradaTeclado - MARGEM_ERRO_TECLADO);
  valorEntradaMaximo = (valorEntradaTeclado + MARGEM_ERRO_TECLADO);

  // Controle de repetição
  if ( valorEntradaTecladoAnterior >= valorEntradaMinimo && valorEntradaTecladoAnterior <= valorEntradaMaximo )
    timeDebouce = timeDebouceAtrasoRepeticao;
  else  
    timeDebouce = timeDebouceAtraso;

  // Controle de atraso para Debouce
  if ((millis() - timeAtualDebouce) > timeDebouce) {
    for(i = 0; i < totalFuncoes; i++) {
      // Procura a função selecionada
      if ( POSICAO_FUNCAO_TECLADO[i] >= valorEntradaMinimo && POSICAO_FUNCAO_TECLADO[i] <= valorEntradaMaximo ) {
        botaoAtualPressionado = i + 1;  
      }
    }
    valorEntradaTecladoAnterior = valorEntradaTeclado;
    timeAtualDebouce = millis();
  }       
}     

void configurarEntradaSaida() {
  // Entradas analógicas
  pinMode(PORTA_TECLADO, INPUT_PULLUP);  
  pinMode(A1, INPUT);

  // Configurando saídas digitais
  for(int i = 0; i < PORTAS_TOTAL; i++){
  	pinMode(PORTAS[i], OUTPUT);
    digitalWrite(PORTAS[i], LOW);
    portasSaidas[i].estado = 0;
    portasSaidas[i].valorAtual = 0;
    portasSaidas[i].modo = 0;
    portasSaidas[i].millisIntervalo = 0;
    portasSaidas[i].millisAtual = 0;
  }     
}

void configurarMenuItems() {
  int item = -1;
  item++; menuItens[item].idNivelRaiz = 0; menuItens[item].idNivelOrigem = 0; menuItens[item].idNivelDestino = 1; menuItens[item].idOrdem = 1; menuItens[item].nome = "Oscilador"; menuItens[item].parametroOpcao = 0; 
  item++; menuItens[item].idNivelRaiz = 0; menuItens[item].idNivelOrigem = 0; menuItens[item].idNivelDestino = 2; menuItens[item].idOrdem = 2; menuItens[item].nome = "Saida PWM"; menuItens[item].parametroOpcao = 0; 
  item++; menuItens[item].idNivelRaiz = 0; menuItens[item].idNivelOrigem = 0; menuItens[item].idNivelDestino = 8; menuItens[item].idOrdem = 3; menuItens[item].nome = "Chaves logicas"; menuItens[item].parametroOpcao = 0;   
  item++; menuItens[item].idNivelRaiz = 0; menuItens[item].idNivelOrigem = 0; menuItens[item].idNivelDestino = 3; menuItens[item].idOrdem = 4; menuItens[item].nome = "Contador binario"; menuItens[item].parametroOpcao = 0;   

  item++; menuItens[item].idNivelRaiz = 1; menuItens[item].idNivelOrigem = 0; menuItens[item].idNivelDestino = 7; menuItens[item].idOrdem = 1; menuItens[item].nome = "%dms"; menuItens[item].parametroOpcao = 100; 
  item++; menuItens[item].idNivelRaiz = 1; menuItens[item].idNivelOrigem = 0; menuItens[item].idNivelDestino = 7; menuItens[item].idOrdem = 2; menuItens[item].nome = "%dms"; menuItens[item].parametroOpcao = 250; 
  item++; menuItens[item].idNivelRaiz = 1; menuItens[item].idNivelOrigem = 0; menuItens[item].idNivelDestino = 7; menuItens[item].idOrdem = 3; menuItens[item].nome = "%dms"; menuItens[item].parametroOpcao = 500;   
  item++; menuItens[item].idNivelRaiz = 1; menuItens[item].idNivelOrigem = 0; menuItens[item].idNivelDestino = 7; menuItens[item].idOrdem = 4; menuItens[item].nome = "%dms"; menuItens[item].parametroOpcao = 1000;     

  item++; menuItens[item].idNivelRaiz = 2; menuItens[item].idNivelOrigem = 0; menuItens[item].idNivelDestino = 0; menuItens[item].idOrdem = 1; menuItens[item].nome = "Desligar todas"; menuItens[item].parametroOpcao = 0; 
  item++; menuItens[item].idNivelRaiz = 2; menuItens[item].idNivelOrigem = 0; menuItens[item].idNivelDestino = 9; menuItens[item].idOrdem = 2; menuItens[item].nome = "Escolher porta"; menuItens[item].parametroOpcao = 0;  

  item++; menuItens[item].idNivelRaiz = 3; menuItens[item].idNivelOrigem = 0; menuItens[item].idNivelDestino = 4; menuItens[item].idOrdem = 1; menuItens[item].nome = "Contar ate X"; menuItens[item].parametroOpcao = 0; 
  item++; menuItens[item].idNivelRaiz = 3; menuItens[item].idNivelOrigem = 0; menuItens[item].idNivelDestino = 5; menuItens[item].idOrdem = 2; menuItens[item].nome = "Dado eletronico"; menuItens[item].parametroOpcao = 0;   

  item++; menuItens[item].idNivelRaiz = 4; menuItens[item].idNivelOrigem = 3; menuItens[item].idNivelDestino = 6; menuItens[item].idOrdem = 1; menuItens[item].nome = "0 ate %d   BCD"; menuItens[item].parametroOpcao = 9; 
  item++; menuItens[item].idNivelRaiz = 4; menuItens[item].idNivelOrigem = 3; menuItens[item].idNivelDestino = 6; menuItens[item].idOrdem = 2; menuItens[item].nome = "0 ate %d  BCD"; menuItens[item].parametroOpcao = 99; 
  item++; menuItens[item].idNivelRaiz = 4; menuItens[item].idNivelOrigem = 3; menuItens[item].idNivelDestino = 6; menuItens[item].idOrdem = 3; menuItens[item].nome = "0 ate %d BIN"; menuItens[item].parametroOpcao = 255;   
  
  item++; menuItens[item].idNivelRaiz = 5; menuItens[item].idNivelOrigem = 3; menuItens[item].idNivelDestino = 0; menuItens[item].idOrdem = 1; menuItens[item].nome = "Gerar ate %d   BCD"; menuItens[item].parametroOpcao = 6; 
  item++; menuItens[item].idNivelRaiz = 5; menuItens[item].idNivelOrigem = 3; menuItens[item].idNivelDestino = 0; menuItens[item].idOrdem = 2; menuItens[item].nome = "Gerar ate %d  BCD"; menuItens[item].parametroOpcao = 99; 
  item++; menuItens[item].idNivelRaiz = 5; menuItens[item].idNivelOrigem = 3; menuItens[item].idNivelDestino = 0; menuItens[item].idOrdem = 3; menuItens[item].nome = "Gerar ate %d BIN"; menuItens[item].parametroOpcao = 255;     

  item++; menuItens[item].idNivelRaiz = 6; menuItens[item].idNivelOrigem = 4; menuItens[item].idNivelDestino = 0; menuItens[item].idOrdem = 1; menuItens[item].nome = "Iniciar/Parar"; menuItens[item].parametroOpcao = 0; 
  item++; menuItens[item].idNivelRaiz = 6; menuItens[item].idNivelOrigem = 4; menuItens[item].idNivelDestino = 0; menuItens[item].idOrdem = 2; menuItens[item].nome = "Reiniciar"; menuItens[item].parametroOpcao = 0;   
  item++; menuItens[item].idNivelRaiz = 6; menuItens[item].idNivelOrigem = 4; menuItens[item].idNivelDestino = 0; menuItens[item].idOrdem = 3; menuItens[item].nome = "Incrementar 1"; menuItens[item].parametroOpcao = 0; 
  item++; menuItens[item].idNivelRaiz = 6; menuItens[item].idNivelOrigem = 4; menuItens[item].idNivelDestino = 0; menuItens[item].idOrdem = 4; menuItens[item].nome = "Subtrair 1"; menuItens[item].parametroOpcao = 0;    

  item++; menuItens[item].idNivelRaiz = 7; menuItens[item].idNivelOrigem = 1; menuItens[item].idNivelDestino = 0; menuItens[item].idOrdem = 1; menuItens[item].nome = "Iniciar/Parar"; menuItens[item].parametroOpcao = 0; 
  item++; menuItens[item].idNivelRaiz = 7; menuItens[item].idNivelOrigem = 1; menuItens[item].idNivelDestino = 0; menuItens[item].idOrdem = 2; menuItens[item].nome = "Desligar tudo"; menuItens[item].parametroOpcao = 0; 

  item++; menuItens[item].idNivelRaiz = 8; menuItens[item].idNivelOrigem = 0; menuItens[item].idNivelDestino = 0; menuItens[item].idOrdem = 1; menuItens[item].nome = "Ligar/Desligar"; menuItens[item].parametroOpcao = 0; 
  item++; menuItens[item].idNivelRaiz = 8; menuItens[item].idNivelOrigem = 0; menuItens[item].idNivelDestino = 0; menuItens[item].idOrdem = 2; menuItens[item].nome = "Desligar tudo"; menuItens[item].parametroOpcao = 0;   

  item++; menuItens[item].idNivelRaiz = 9; menuItens[item].idNivelOrigem = 2; menuItens[item].idNivelDestino = 0; menuItens[item].idOrdem = 1; menuItens[item].nome = "Diminuir"; menuItens[item].parametroOpcao = 0; 
  item++; menuItens[item].idNivelRaiz = 9; menuItens[item].idNivelOrigem = 2; menuItens[item].idNivelDestino = 0; menuItens[item].idOrdem = 2; menuItens[item].nome = "Aumentar"; menuItens[item].parametroOpcao = 0;        
}

void atualizarDisplay() {    
  int linha = 0, i;
  String portas;
  char buffer [20];

  if (!atualizarLCD) return;

  atualizarLCD = false;

  lcd.clear();

  for(i = 0; i < MENU_ITENS; i++) {
    if ( menuItens[i].idNivelRaiz == idMenuNivelRaizAtual ) {           
      sprintf(buffer, menuItens[i].nome, menuItens[i].parametroOpcao);
      print(linha, (String)menuItens[i].idOrdem + "->" + (String)buffer); 
      linha++;
    }
  }

  if ( atualEstadoMaquina == ES_OSCILADOR      && idMenuNivelRaizAtual == 7 
    || atualEstadoMaquina == ES_SAIDA_PWM      && idMenuNivelRaizAtual == 2 
    || atualEstadoMaquina == ES_CHAVES_LOGICAS && idMenuNivelRaizAtual == 8 ) {
    print(2, "<<-(3) Saida (4)->>");
    
    portaPosicao = (botaoAtualPressionado == 3 && portaPosicao > 0 ? portaPosicao - 1 : portaPosicao);
    portaPosicao = (botaoAtualPressionado == 4 && portaPosicao < 7 ? portaPosicao + 1 : portaPosicao);

    portas = "";    
    for(i = 0; i < 8; i++) {
      portas = portas + (i==portaPosicao ? "["+String(i+1)+"]" : (i < portaPosicao ? "." : "") + String(i+1) + (i > portaPosicao ? "." : "") );
    }    
    print(3, " " + portas);
  }  

  if ( atualEstadoMaquina == ES_SAIDA_PWM && idMenuNivelRaizAtual == 9 ) {
    print(2, "Porta " +(String)(portaPosicao+1)+ " PWM: " + (String)portasSaidas[portaPosicao].valorAtual );

    print(3,"");
    for(i = 0; i < map(portasSaidas[portaPosicao].valorAtual, 0, 255, 0, 20); i++) {
      lcd.setCursor(i, 3);
      lcd.write(byte(0));
    }        
  }    

  if ( atualEstadoMaquina == ES_DADO_ELETRONICO && idMenuNivelRaizAtual == 5 ) {
    print(3,"");    
    if ( posicaoContadorAtual == 0 && numeroDadoEletronicoAtual > 0 ) {
      print(3, "    Numero = " + (String)numeroDadoEletronicoAtual );
      for(i = 0; i < 5;i++) {
        lcd.noBacklight();      
        delay(100);
        lcd.backlight();
        delay(100);
      }
    }
  }   
}

void print(int row, String message) {    
  lcd.setCursor(0, row);
  for(int i = 0; i < LCD_COLUNAS; i++) lcd.print(" ");        
  lcd.setCursor(0,row);
  lcd.print(message);  
}

void atualizarPosicaoMenu() {
  int i, estadoMaquinaAnterior, idMenuNivelRaizAnterior;

  if ( botaoAtualPressionado == 0 ) return;

  estadoMaquinaAnterior = atualEstadoMaquina;
  idMenuNivelRaizAnterior = idMenuNivelRaizAtual;

  for(i = 0; i < MENU_ITENS; i++) {
    if ( menuItens[i].idNivelRaiz == idMenuNivelRaizAtual && (menuItens[i].idOrdem == botaoAtualPressionado || botaoAtualPressionado == 5) ) {    
      valorParametroMenu   = ( botaoAtualPressionado != 5 && menuItens[i].parametroOpcao > 0 || idMenuNivelRaizAtual == 0 ?  menuItens[i].parametroOpcao : valorParametroMenu );        
      atualEstadoMaquina   = ( idMenuNivelRaizAtual == 0 && botaoAtualPressionado != 5 ? botaoAtualPressionado : atualEstadoMaquina); // Define uma das quatro opções do menu principal de 1 a 4
      atualEstadoMaquina   = ( menuItens[i].idNivelOrigem == 0 && botaoAtualPressionado == 5 ? 0 : atualEstadoMaquina); // Volta ao estado ocioso item 0
      atualEstadoMaquina   = ( menuItens[i].idNivelDestino == 4 && botaoAtualPressionado == 1 ? 4 : atualEstadoMaquina); // Define binario item 4      
      atualEstadoMaquina   = ( menuItens[i].idNivelDestino == 5 && botaoAtualPressionado == 2 ? 5 : atualEstadoMaquina); // Define dado eletronico item 5
      idMenuNivelRaizAtual = ( botaoAtualPressionado != 5 && menuItens[i].idNivelDestino > 0 ?  menuItens[i].idNivelDestino : idMenuNivelRaizAtual );      
      idMenuNivelRaizAtual = ( botaoAtualPressionado == 5 ? menuItens[i].idNivelOrigem : idMenuNivelRaizAtual );     

      if ( estadoMaquinaAnterior != atualEstadoMaquina || idMenuNivelRaizAnterior != idMenuNivelRaizAtual ) botaoAtualPressionado = 0;                        

      break;
    }    
  }  
}

void atualizaValorPorta() {  
  char BCD[2];

  if ( atualizarLCD ) {

    if ( atualEstadoMaquina == ES_OSCILADOR && idMenuNivelRaizAtual == 7 ) { // Oscilador
      if ( botaoAtualPressionado == 2 ) desligarPortas(); 
      if ( botaoAtualPressionado < 3 ) {
        portasSaidas[portaPosicao].modo = ST_OSCILADOR;
        portasSaidas[portaPosicao].estado = (botaoAtualPressionado == 1 && portasSaidas[portaPosicao].estado == 0 ? 1 : 0);        
        portasSaidas[portaPosicao].millisIntervalo = (botaoAtualPressionado == 1 ? valorParametroMenu : portasSaidas[portaPosicao].millisIntervalo);   
      }             
    }     

    if ( atualEstadoMaquina == ES_SAIDA_PWM ) { // PWM
      if ( idMenuNivelRaizAtual == 9 && botaoAtualPressionado < 3 ) {
        portasSaidas[portaPosicao].modo = ST_PWM;
        portasSaidas[portaPosicao].valorAtual = (botaoAtualPressionado == 1 && portasSaidas[portaPosicao].valorAtual >= 0 ? portasSaidas[portaPosicao].valorAtual - 10 : portasSaidas[portaPosicao].valorAtual);
        portasSaidas[portaPosicao].valorAtual = (botaoAtualPressionado == 2 && portasSaidas[portaPosicao].valorAtual < 256 ? portasSaidas[portaPosicao].valorAtual + 5 : portasSaidas[portaPosicao].valorAtual);   
        portasSaidas[portaPosicao].valorAtual = (portasSaidas[portaPosicao].valorAtual < 0 ? 0 : portasSaidas[portaPosicao].valorAtual);             
        portasSaidas[portaPosicao].valorAtual = (portasSaidas[portaPosicao].valorAtual > 255 ? 255 : portasSaidas[portaPosicao].valorAtual);             
      } else if ( idMenuNivelRaizAtual == 2 && botaoAtualPressionado == 1) desligarPortas();      
    }     

    if ( atualEstadoMaquina == ES_CHAVES_LOGICAS && idMenuNivelRaizAtual == 8 ) { // Chaves lógicas
      if (botaoAtualPressionado == 2) desligarPortas();        
      if ( botaoAtualPressionado < 3 ) {
        portasSaidas[portaPosicao].modo = ST_LOGICO;
        portasSaidas[portaPosicao].estado = (botaoAtualPressionado == 1 && portasSaidas[portaPosicao].estado == 0 ? 1 : 0);
        portasSaidas[portaPosicao].valorAtual = portasSaidas[portaPosicao].estado;
      }      
    }             
  } 

  if ( atualEstadoMaquina == ES_CONTADOR_BINARIO ) { // Contador binário
    if ( idMenuNivelRaizAtual == 3 ) {                
      contadorAtivo = false;
      posicaoContadorAtual = 0;
      valorParametroMenu = 0;
      desligarPortas();        
    }
    if ( idMenuNivelRaizAtual == 4 ) {                 
      contadorAtivo = false;
      posicaoContadorAtual = 0;        
      valorParametroMenu = 0;    
    }       
    if ( atualizarLCD && idMenuNivelRaizAtual == 6 ) {
      if ( botaoAtualPressionado == 1 ) contadorAtivo = ( contadorAtivo ? 0 : 1);
      intervaloMilisContador = ( contadorAtivo ? intervaloMilisContadorBase - valorParametroMenu : intervaloMilisContador); 
      posicaoContadorAtual = (botaoAtualPressionado == 2  ? 0 : posicaoContadorAtual);
      posicaoContadorAtual = (botaoAtualPressionado == 3 && !contadorAtivo ? posicaoContadorAtual + 1 : posicaoContadorAtual);
      posicaoContadorAtual = (botaoAtualPressionado == 4 && !contadorAtivo ? posicaoContadorAtual - 1 : posicaoContadorAtual);    
      posicaoContadorAtual = ( posicaoContadorAtual < 0 ? valorParametroMenu : posicaoContadorAtual );
      posicaoContadorAtual = ( posicaoContadorAtual > valorParametroMenu ? 0 : posicaoContadorAtual );   
      sprintf(BCD, "%02d", posicaoContadorAtual);               
    }   
  }  

  if ( atualEstadoMaquina == ES_DADO_ELETRONICO && ( idMenuNivelRaizAtual == 5 || idMenuNivelRaizAtual == 3 ) ) { // Dado eletrônico      
    if ( atualizarLCD && botaoAtualPressionado < 4 ) {
      posicaoContadorAtual = 30; // Randomiza vários números até o limite
      numeroDadoEletronicoAtual = 0;
      intervaloMilisContador = 50;
      timeAnteriorContador = millis() + intervaloMilisContador + 1;
      desligarPortas();
    } 
    if ( atualizarLCD && idMenuNivelRaizAtual == 3 ) { 
      numeroDadoEletronicoAtual = 0;
      valorParametroMenu = 0;
      posicaoContadorAtual = 0;       
      desligarPortas(); 
    }
    if ( (millis() - timeAnteriorContador) > intervaloMilisContador && valorParametroMenu > 0 && posicaoContadorAtual > 0) {    
      posicaoContadorAtual = ( posicaoContadorAtual > 0 ? posicaoContadorAtual - 1 : 0 );  
      numeroDadoEletronicoAtual = ( idMenuNivelRaizAtual == 5 ? random(1, valorParametroMenu) : 0 );
      atualizarLCD = ( posicaoContadorAtual == 0 ? true : atualizarLCD );
      timeAnteriorContador = millis();
    }       
    sprintf(BCD, "%02d", numeroDadoEletronicoAtual); 
  }  

  if ( atualEstadoMaquina == ES_CONTADOR_BINARIO && idMenuNivelRaizAtual == 6 && contadorAtivo && valorParametroMenu > 0 ) {
    if ( (millis() - timeAnteriorContador) > intervaloMilisContador ) {    
      posicaoContadorAtual++;       
      if ( posicaoContadorAtual > valorParametroMenu )  posicaoContadorAtual = 0;      
      timeAnteriorContador = millis();
    }            
    sprintf(BCD, "%02d", posicaoContadorAtual);
  }    

  for(int i = 0; i < 8; i++) {
    if ( portasSaidas[i].modo == ST_OSCILADOR ) {
      if ( portasSaidas[i].millisIntervalo > 0 && (millis() - portasSaidas[i].millisAtual) > portasSaidas[i].millisIntervalo ){                         
        portasSaidas[i].valorAtual = (portasSaidas[i].estado > 0 ? (portasSaidas[i].valorAtual == 1 ? 0 : 1) : 0);
        portasSaidas[i].millisAtual = millis();
      }
    }

    // Atualiza portas de saida    
    if ( atualEstadoMaquina < ES_CONTADOR_BINARIO && ( portasSaidas[i].modo == ST_OSCILADOR || portasSaidas[i].modo == ST_LOGICO) )    
      digitalWrite(PORTAS[i], portasSaidas[i].valorAtual);
    else if ( atualEstadoMaquina < ES_CONTADOR_BINARIO && portasSaidas[i].modo == ST_PWM )
      analogWrite(PORTAS[i], portasSaidas[i].valorAtual);
    else if ( atualEstadoMaquina == ES_CONTADOR_BINARIO ) {
      if ( valorParametroMenu == 99 ) {
        if ( i < 4 )
          digitalWrite(PORTAS[i], (bitRead((int)BCD[1], i) == 0 ? 0 : 1 ) );   
        else
          digitalWrite(PORTAS[i], (bitRead((int)BCD[0], i - 4) == 0 ? 0 : 1 ) );         
      } else {
        digitalWrite(PORTAS[i], (bitRead(posicaoContadorAtual, i) == 0 ? 0 : 1 ) );
      }              
    } else if ( atualEstadoMaquina == ES_DADO_ELETRONICO ) {
      if ( valorParametroMenu == 99 ) {
        if ( i < 4 )
          digitalWrite(PORTAS[i], (bitRead((int)BCD[1], i) == 0 ? 0 : 1 ) );   
        else
          digitalWrite(PORTAS[i], (bitRead((int)BCD[0], i - 4) == 0 ? 0 : 1 ) );         
      } else {
        digitalWrite(PORTAS[i], (bitRead(numeroDadoEletronicoAtual, i) == 0 ? 0 : 1 ) );
      }  
    }
  }
}

void desligarPortas() {
  for(int i = 0; i < 8; i++) {
    portasSaidas[i].estado = 0;
    portasSaidas[i].valorAtual = 0;    
    digitalWrite(PORTAS[i], LOW);
  }  
}

