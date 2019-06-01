/*********
  PROGRAMA PARA CONTROLAR VÁLVULAS SOLENÓIDES REMOTAMENTE.
*********/

// Carregar biblioteca WIFI:
#include <ESP8266WiFi.h>

// Credenciais da rede:
const char* ssid     = "VNR_NET";
const char* password = "vnr71972";

// Configurar porta do servidor web para 80:
WiFiServer server(80);

// Variável para armazenar a solicitação do HTTP:
String header;

// Variável auxiliar para armazenar o estado atual de saída:
String outputState = "aberta";

// Atribuir variáveis ​​de saída aos pinos GPIO:
const int output = 2;
const int ledVerde = 4;

void setup() {
  Serial.begin(9600);
  // Inicializar as variáveis ​​de saída como saídas:
  pinMode(output, OUTPUT);
  pinMode(ledVerde, OUTPUT);
  // Definir saídas como Low:
  digitalWrite(output, LOW);
  digitalWrite(ledVerde, HIGH);

  // Conectar ao WIFI
  Serial.print("Conectando em ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Mensagem de conexão bem sucedida e endereço IP:
  Serial.println("");
  Serial.println("WiFi conectado.");
  Serial.println("Endereço IP: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

void loop(){
  WiFiClient client = server.available();   // Esperando o ingresso de novos clientes;

  if (client) {                             // Se um novo cliente conecta;
    Serial.println("New Client.");          // Imprimir uma mensagem na porta serial;
    String currentLine = "";                // Cria uma String para manter os dados recebidos do cliente;
    while (client.connected()) {            // Repetição enquanto o cliente está conectado;
      if (client.available()) {             // Se há bytes para ler do cliente;
        char c = client.read();             // Ler o byte, em seguida;
        Serial.write(c);                    // Imprimir o monitor serial;
        header += c;
        if (c == '\n') {                    // Se o byte é o caractere de nova linha;
          // Se a linha atual estiver em branco, você tem dois caracteres de nova linha em uma linha.
          // Esse é o fim da solicitação HTTP do cliente, então envie uma resposta:
          if (currentLine.length() == 0) {
            // Cabeçalhos HTTP sempre começam com um código de resposta (e.g. HTTP/1.1 200 OK);
            // E um tipo de conteúdo para que o cliente saiba o que está por vir, depois uma linha em branco:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // Controlar o GPIO:
            if (header.indexOf("GET /2/on") >= 0) {
              Serial.println("GPIO 2 on");
              outputState = "Fechada";
              digitalWrite(output, HIGH);
              digitalWrite(ledVerde, LOW);
            } else if (header.indexOf("GET /2/off") >= 0) {
              Serial.println("GPIO 2 off");
              outputState = "Aberta";
              digitalWrite(output, LOW);
              digitalWrite(ledVerde, HIGH);
            } 
            
            // Display da página HTML:
            client.println("<!DOCTYPE html><html>");
            client.println("<head>");
            client.println("<meta charset=\"utf-8\">");
            client.println("<link rel=\"shortcut icon\" href=\"https://i.ibb.co/Srb6BVN/pp.jpg\" type=\"image/x-icon\" />");
            client.println("<title>SIGA UFCG</title>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"https://ibb.co/gVNrJty\"><img width=\"150px\" height=\"150px\" src=\"https://i.ibb.co/Srb6BVN/pp.jpg\" alt=\"Siga UFCG\" border=\"0\">");
            // CSS do estilo dos botões de ligar/desligar:
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #12ADFF; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #77878A;}</style></head>");
            
            // Título da página:
            client.println("<body bgcolor=\"D3D3D3\" font color=\"#12ADFF\"><h1>CONTROLE DE PASSAGEM DE ÁGUA</h1>");
            
            // Estado atual do display, e botões abrir/fechar do GPIO 2:
            client.println("<p>Válvula " + outputState + "</p>");
            // Se o outputState é aberto, é inserido o botão fechar no display:
            if (outputState=="Aberta") {
              client.println("<p><a href=\"/2/on\"><button class=\"button\">Fechar</button></a></p>");
            } else {
              client.println("<p><a href=\"/2/off\"><button class=\"button button2\">Abrir</button></a></p>");
            } 

            // A resposta do HTTP termina com outra linha em branco:
            client.println();
            // Quebrando a estrutura de repetição:
            break;
          } else { // Se há uma nova linha, deixar ela em branco:
            currentLine = "";
          }
        } else if (c != '\r') {  // Se você tem algo além de um caractere de retorno;
          currentLine += c;      // Adicione-o ao final da linha atual;
        }
      }
    }
    // Limpar a variável de cabeçalho:
    header = "";
    // Desconectar:
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
