/*==================================================================
 * Projeto: Radar Inteligente
 * Autor: Igo L. S. Rabelo
 * Última atualização: 28/12/2020
 =================================================================*/

//inclui as bibliotecas para controlar o display e o servo o e controle infravermelho
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <IRremote.h>

//cria um objeto chamado lcd da classe LiquidCrystal_I2C
LiquidCrystal_I2C lcd(0x27,20,4);
//cria um objeto chamado motor da classe Servo
Servo motor;

/*=================================================================
 * 
 * 
 *                  mapeamento de hardware
 ================================================================*/
#define ldr A2
#define led_verm 10
#define led_verde 11
#define Receptor 12
#define buzzer 2
#define botaoStart 9
#define pinTrig 6
#define pinEcho 7
IRrecv irrecv(Receptor);

/*==================================================================
                        Variáveis globais
 =================================================================*/

int inten_luz;                                  //variável que armazena a intensidade de luz lido pelo sensor
float valorDistancia, duracaoPulso, distancia;  //variável que armazena a duracao de tempo do pulso e o valor da distancia ja calculada
int estadoSensor = 0;                           //variável que armazena o estado do sensor, se nada for indentificado, "estadoSensor = 0"
int estado_btn = LOW;                           //variável de controle que armazena o estado do botão
int estado_ant = LOW;                           //variável que armazena o estado anterior do botão que liga o sistema 
int ligar = LOW;                                //variável que liga o sistema 
decode_results resultado;                       //variável que armazena o código lido pelo receptor de infravermelho

void setup() {
   
  lcd.init();                    //inicializa o display
  lcd.backlight();               //liga o backlight do display 
  Serial.begin(115200);          //inicia a comunicação serial
  motor.attach(14);              //atribui o pino 4 como pino de sinal para o servo
  irrecv.enableIRIn();           //habilita o receptor de infravermelho 
  lcd.clear();                   //limpa o display 
  
  pinMode(led_verde,OUTPUT);
  pinMode(led_verm,OUTPUT);
  pinMode(buzzer,OUTPUT);
  pinMode(botaoStart,INPUT_PULLUP);
  pinMode(pinTrig,OUTPUT);
  pinMode(pinEcho,INPUT);
  estadoSensor = 0;             
}

void loop() {

  estado_btn = digitalRead(botaoStart); 
  valorDistancia = calcDistancia();                //Chama a função que calcula a distância a parti da leitura do sensor ultrassônico
  inten_luz = analogRead(ldr);                     //armazena o valor lido pela LDR via porta analógica
  
  //se o sensor detectar algum objeto a menos de 20cm dele, chama a função obj_detectado()
  if (valorDistancia < 20) 
     {   
      obj_detectado();
     }
     else {
      //se a distancia lida pelo sensor não for menor do que 20cm, mantêm a variável estadoSensor = 0;
      //indicando que nenhum objeto foi detectado.
      estadoSensor = 0;               
     }
     //verifica se alguma sinal foi recebido pelo receptor de infravermelho
     if(irrecv.decode(&resultado)) 
  {
   Serial.println(resultado.value, HEX);
   irrecv.resume();
  }
  
//se o botão for pressionado ou a tecla 1 do controle for pressionada, inverte o estado da variável "ligar"
if((estado_btn == 0 && estado_ant == 1) || resultado.value == 0xFFA25D) 
{
  ligar = !ligar;

  //faz o buzzer tocar 7 vezes
  for (int tocaBuzzer = 0; tocaBuzzer <= 7; tocaBuzzer++)
      {
        digitalWrite(buzzer,1);
        delay(20);
        digitalWrite(buzzer,0);
        delay(5);     
      }
     //faz o LED verde piscar 7 vezes 
     for (int pistaLed = 0; pistaLed <= 7; pistaLed++)
      {
       digitalWrite(led_verde,1);
       delay(45);
       digitalWrite(led_verde,0);
       delay(25);   
      }
      //limpa o display
      lcd.setCursor(0,0);
      lcd.print("                    ");
      lcd.setCursor(0,1);
      lcd.print("                    ");
      lcd.setCursor(0,2);
      lcd.print("                    ");
      lcd.setCursor(0,3);
      lcd.print("                    ");
      lcd.setCursor(1,1);
      lcd.backlight();
      lcd.print("Sistema Iniciado!");
      delay(700);    
}
   //"tela" inicial do display enquanto o usuário que inicia o sistema  
   if (valorDistancia > 20){ 
     
      lcd.backlight();
      lcd.setCursor(0, 0);
      lcd.print(" ");
      lcd.setCursor(19, 0);
      lcd.print("  ");
      lcd.setCursor(2, 1);
      lcd.print(" ");
      lcd.setCursor(16, 1);
      lcd.print("  ");
      lcd.setCursor(1, 0);
      lcd.print("Radar Inteligente");
      lcd.setCursor(3, 1);
      lcd.print("Por Igo Levir");
      lcd.setCursor(0, 2);
      lcd.print("Tecle 1 ou pressione");
      lcd.setCursor(0, 3);
      lcd.print("o botao para iniciar");
     }
       //uma vez que o sistema é acionado o estado da variável "estado_ant" é invertido 
       estado_ant = estado_btn;
       
    //Se o botão start ou a tecla 1 for pressionado e não tiver nenhum objeto a menos de 20cm do sensor,
    //o sistema começa a varredura e exibe uma mensagem no lcd para o usuário   
    if(ligar == 1 && inten_luz < 90 && estadoSensor == 0 && valorDistancia > 20)
   { 
    SistAcionado();              //função de que aciona o sistema  
   }
}
 //função que informa para o usuário quando um objeto foi detectado
 void obj_detectado()
  {
      estadoSensor = 1;
      digitalWrite(buzzer,1);
      digitalWrite(led_verm,1);
      delay(80);
      digitalWrite(buzzer,0);
      digitalWrite(led_verm,0);
      delay(80);
      lcd.backlight();
      lcd.setCursor(0,0);
      lcd.print("                    ");
      lcd.setCursor(0,1);
      lcd.print("  ");
      lcd.setCursor(0,2);
      lcd.print("                    ");
      lcd.setCursor(0,3);
      lcd.print("                    ");
      lcd.setCursor(2,1);
      lcd.print("Objeto detectado");
      delay(100);
      lcd.noBacklight();
      delay(10);    
  }
 //função que calcula a distancia do sensor até objetos dentro do campo de visão 
 float calcDistancia()                  
  {
    digitalWrite(pinTrig,0);    
    delayMicroseconds(5);
    digitalWrite(pinTrig,1);
    delayMicroseconds(10);
    duracaoPulso = pulseIn(pinEcho,1);
    distancia = (duracaoPulso/2)*0.0343;
    return distancia;    
  }
/*=================================================================
                 Função que aciona o sistema
=================================================================*/
 int SistAcionado()
  {  
    for (int angulo = 0; angulo <= 180; angulo=angulo+6)
      {
       //enquando o sensor estiver identificando objetos, mantem o usuário informado que algo foi detectado   
       while (valorDistancia < 20)
        {
          lcd.setCursor(0,0);
          lcd.print("                    ");
          lcd.setCursor(0,1);
          lcd.print("                    ");
          lcd.setCursor(0,2);
          lcd.print("                    ");
          lcd.setCursor(1,0);
          lcd.print("Objeto detectado");
          lcd.setCursor(3,1);
          lcd.print("em ");
          lcd.print(angulo);
          lcd.print(" graus");
          delay(1000);
          return 0;
          break;
          }
     //chama a função que calcula distancia, faz o servo girar de 0º a 180º e exibe o            
     // angulo varido pelo sensor no display   
     motor.write(angulo);
     valorDistancia = calcDistancia();
     //limpa o display e exibe o angulo varido pelo sensor
     lcd.setCursor(0,0);
     lcd.print("                    ");
     lcd.setCursor(0,1);
     lcd.print("   ");
     lcd.setCursor(0,2);
     lcd.print("     ");
     lcd.setCursor(14,2);
     lcd.print("      ");
     lcd.setCursor(0,3);
     lcd.print("                    ");
     lcd.setCursor(8,1);
     lcd.print(" ");
     lcd.setCursor(3, 1); 
     lcd.print("Angulo atual:"); 
     lcd.setCursor(5,2);
     lcd.print(angulo);
     lcd.setCursor(8,2);
     lcd.print(" ");
     lcd.setCursor(9,2);
     lcd.print("graus"); 
        
      }
    for(int angulo = 180; angulo >= 0; angulo=angulo-6)
      {
       //enquando o sensor estiver identificando objetos, mantem o usuário informado que algo foi detectado
       while (valorDistancia < 19)
        {        
          lcd.setCursor(0,0);
          lcd.print("                    ");
          lcd.setCursor(0,1);
          lcd.print("                    ");
          lcd.setCursor(0,2);
          lcd.print("                    ");
          lcd.setCursor(1,0);
          lcd.print("Objeto detectado");
          lcd.setCursor(3,1);
          lcd.print("em ");
          lcd.setCursor(2,1);
          lcd.print(angulo);
          lcd.print(" graus");
          delay(1000);
          return 0;
          break;
        }
     //chama a função que calcula a distancia, faz o servo girar de 0º a 180º e exibe
     //o angulo varido pelo sensor no display    
     valorDistancia = calcDistancia();   
     motor.write(angulo);
     //limpa o display e exibe o angulo varido pelo sensor
     lcd.setCursor(0,0);
     lcd.print("                    ");
     lcd.setCursor(0,1);
     lcd.print("   ");
     lcd.setCursor(0,2);                                    
     lcd.print("     ");
     lcd.setCursor(14,2);
     lcd.print("      ");
     lcd.setCursor(0,3);
     lcd.print("                    ");
     lcd.setCursor(8,1);
     lcd.print(" ");
     lcd.setCursor(3, 1);
     lcd.print("Angulo atual:");
     lcd.setCursor(5,2); 
     lcd.print(angulo);
     lcd.setCursor(8,2);
     lcd.print(" ");
     lcd.setCursor(9,2);
     lcd.print("graus");
        }         
 }
