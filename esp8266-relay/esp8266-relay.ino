// Exercício: Acionamento de relê na Wifi com ESP8266
// Aluno: Gustavo de Souza Silveira - 05/08/2021 - @gustavus.dev
// Prof. Nelson Garcia - UTFPR-CT

// Carrega biblioteca wifi
#include <ESP8266WiFi.h>

// Servidor web na porta 80
WiFiServer server(80);

// Rede Wifi e senha
const char* ssid     = "nome-da-rede-wifi";
const char* password = "senha-da-wifi";


// Controle da saída GPIO 2 e TX
String estadoSaidaGPIO2 = "desligar";
const int saidaGPIO2 = 2;
const int saidaTX = 1;

// Variavel da requisição HTTP
String header;

// Controle de tempo
unsigned long tempoAtual = millis();
unsigned long tempoAnterior = 0; 
const long tempoLimite = 2000; 

String estado ="";

void setup() {
  
  //Serial.begin(115200);
  
  // Saída de controle
  pinMode(saidaGPIO2, OUTPUT);

  pinMode(saidaTX, FUNCTION_3); // Configura TX como saída para controlar o acionamento em nível alto da GPIO2
  
  // Alterando o nível lógico das saídas  
  digitalWrite(saidaGPIO2, HIGH);
  digitalWrite(saidaTX, LOW); // Desliga o fotoacoplador durante boot do ESP01 para evitar repique da GPIO2

  // Conectando na rede sem fio
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); // Aguardando conexão  
  }
  //Serial.println(WiFi.localIP());
  delay(1000);
  digitalWrite(saidaTX, HIGH); // Liga o fotoacoplador
  server.begin();
}

void loop(){
  WiFiClient client = server.available();   // Escuta a entrada de clientes

  if (client) {                             // Cliente conectado    
    String linhaAtual = "";                
    tempoAtual = millis();
    tempoAnterior = tempoAtual;
    while (client.connected() && tempoAtual - tempoAnterior <= tempoLimite) { // loop do cliente conectado
      tempoAtual = millis();         
      if (client.available()) {             // se houver bytes para ler do cliente,
        char c = client.read();             // Lê os bytes        
        header += c;
        if (c == '\n') {                    // se o byte é um caractere de nova linha 
          // se a linha atual estiver em branco, você terá dois caracteres de nova linha em uma linha. 
          // esse é o fim da solicitação HTTP do cliente, então envie uma resposta: 
          if (linhaAtual.length() == 0) {
            // Os cabeçalhos HTTP sempre começam com um código de resposta  (e.g. HTTP/1.1 200 OK)
            // e um tipo de conteúdo para que o cliente saiba o que está por vir, em seguida, uma linha em branco: 
            
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // Controle das GPIOs
            if (header.indexOf("GET /2/ligar") >= 0) {
              Serial.println("GPIO 4 on");
              estadoSaidaGPIO2 = "ligar";
              digitalWrite(saidaGPIO2, LOW);
            } else if (header.indexOf("GET /2/desligar") >= 0) {
              Serial.println("GPIO 4 off");
              estadoSaidaGPIO2 = "desligar";
              digitalWrite(saidaGPIO2, HIGH);
            }
            
            // Exibe o HTML da página web
            client.println("<!DOCTYPE html><html>");
            client.println("<head>");
            client.println("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            
            client.println("<style>");
            client.println(" body {  align-items: center;    background-color: #000;    display: flex;    justify-content: center;    height: 100vh;  } ");
            client.println(" .form {  background-color: #15172B;    border-radius: 12px;    box-sizing: border-box;    height: 320px;    padding: 10px;    width: 320px;  } "); 
            client.println(" .title {    color: #eee;    font-family: sans-serif;    font-size: 32px;    font-weight: 600;    margin-top: 15px;  } "); 
            client.println(" .subtitle {    color: #eee;    font-family: sans-serif;    font-size: 16px;    font-weight: 600;    margin-top: 10px;  } "); 
            
            client.println(" .button { background-color: #08d; border-radius: 12px; border: 0; box-sizing: border-box; color: #eee; cursor: pointer; font-size: 18px; height: 50px; margin-top: 38px; text-align: center; width: 100%;  }");
            client.println(" .button.ativo { background-color: #06b; }");
            client.println(" .button.inativo { background-color: #00AABB; }");            
            client.println(" text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println("</style>");
            client.println("</head>");
            
            // Cabeçalho da página
            client.println("<body><div class='form'>");
            client.println("<div class='title'>ESP8266</br>Web Server</div><div class='subtitle'>@gustavus.dev</div>");
            
            // Exibe o botão de liga e desliga
            estado = (estadoSaidaGPIO2 == "ligar" ? "ligado" : "desligado" );
            client.println("</br></br><div class='subtitle'>GPIO 2 - Estado <b>" + estado + "</b></div>");             
            if (estadoSaidaGPIO2=="desligar") {
              client.println("<p><a href=\"/2/ligar\"><button class=\"button ativo\">LIGAR</button></a></p>");
            } else {
              client.println("<p><a href=\"/2/desligar\"><button class=\"button inativo\">DESLIGAR</button></a></p>");
            }
            client.println("</div></body></html>");
            
            // A resposta HTTP termina com outra linha em branco 
            client.println();            
            break;
          } else { 
            linhaAtual = "";
          }
        } else if (c != '\r') { 
          linhaAtual += c;      
        }
      }
    }     
    header = "";   // Limpa o cabeçalho
    client.stop(); // Fecha a conexão
  }
}
