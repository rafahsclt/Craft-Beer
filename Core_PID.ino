#include <TimerOne.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define liga_resistencia      digitalWrite(resistencia,HIGH)
#define desliga_resistencia   digitalWrite(resistencia,LOW)
#define liga_bomba            digitalWrite(bomba,HIGH) 
#define desliga_bomba         digitalWrite(bomba,LOW) 
#define liga_misturador       digitalWrite(misturador,HIGH) 
#define desliga_misturador    digitalWrite(misturador,LOW) 
#define resistencia           7
#define bomba                 3                     // Estado da bomba
#define misturador            4                     // Estado do misturador               
#define tolerancia            0.5                   // Tolerancia de temperatura

OneWire pino(9);
DallasTemperature barramento(&pino);
DeviceAddress sensor;

unsigned char volatile potencia = 0;                // Valor em potência

unsigned char pot_controlada = 0,                   // Valor de potência controlado pelo usuário
              led = 0;

unsigned int cont_tempo = 0;                       // Contado de tempo (1seg)

float valor_sensor,                                 // Valor do sensor
      temp_sp = 0;                                  // Valor setpoint de temperatura

bool pid_flag = true,
     mosturacao = false,
     fervura = false,
     histerese = true;

String cmd = "";                                    // Variável que recebe string

void setup()
{ 
pinMode(resistencia,OUTPUT);
pinMode(bomba,OUTPUT);
pinMode(misturador,OUTPUT);
desliga_resistencia;
desliga_bomba;
desliga_misturador;
Timer1.initialize(1000); // Inicializa o Timer1 e configura para um período de 1 milisegundo
Timer1.attachInterrupt(timer1ms); // Configura a função callback() como a função para ser chamada a cada interrupção do Timer1
Serial.begin(9600);
barramento.begin();
barramento.getAddress(sensor, 0); 
}



void timer1ms()
{
static unsigned char tempo10ms = 10;
static unsigned char tempo1s = 100;

ControlaResistencia();                          // Chama função que controla a cada 1ms              

  if(tempo10ms)
  {
  tempo10ms--;
  }
  else
  {
  tempo10ms = 10;
  
    if(tempo1s)
    {
    tempo1s--;
    }
    else
    {
    tempo1s = 100;
    cont_tempo++;
    LeituraTemperatura();
    if(mosturacao == true) Mosturacao();
    if(fervura == true) Fervura();
    } 
  }
}

void Mosturacao()
{
   Serial.print(potencia);
   Serial.print(",");
   Serial.print(led);
   Serial.print(",");
   Serial.print(cont_tempo);
   Serial.print(",");
   Serial.print(valor_sensor);
   Serial.println(";"); 
   if(valor_sensor < (temp_sp - tolerancia) )
   {
    potencia = pot_controlada;
    liga_misturador;
    led = 1;
    histerese = true;
   }
   else 
   {
      if (valor_sensor < (temp_sp + tolerancia) && histerese == true)
      {
      potencia = pot_controlada;
      liga_misturador;
      led = 1;
      }
      else
      {
      potencia = 0;
      desliga_misturador;
      led = 0;
      histerese = false;
      }
   } 
}

void Fervura()
{
 potencia = pot_controlada;
 Serial.print(potencia);
 Serial.print(",");
 Serial.print(led);
 Serial.print(",");
 Serial.print(cont_tempo);
 Serial.print(",");
 Serial.print(valor_sensor);
 Serial.println(";");
}

void LeituraTemperatura()
{
barramento.requestTemperatures(); 
valor_sensor = barramento.getTempC(sensor);
}


void loop()
{
  if (Serial.available())
  {
    cmd = Serial.readString();
    if(cmd.substring(0,3) == "TMP")   temp_sp = cmd.substring(3,5).toInt();
    if(cmd.substring(0,3) == "POT")   pot_controlada = cmd.substring(3,5).toInt();
    if(cmd.substring(0,5) == "LIGAB") liga_bomba;
    if(cmd.substring(0,5) == "DSLGB") desliga_bomba;
    if(cmd.substring(0,5) == "LIGAM") liga_misturador;
    if(cmd.substring(0,5) == "DSLGM") desliga_misturador;
    if(cmd.substring(0,5) == "LIGAR") liga_resistencia;
    if(cmd.substring(0,5) == "DSLGR") desliga_resistencia;
    if(cmd.substring(0,5) == "ZRCNT") cont_tempo = 0;
    //if(cmd.substring(0,5) == "MANUAL")  pid_flag = false;
    if(cmd.substring(0,5) == "TRFRC")
    {
      mosturacao = false;
      desliga_misturador;
      desliga_bomba;
      potencia = 0;
    }
    if(cmd.substring(0,5) == "FIMFV")
    {
      fervura = false;
      potencia = 0;
    }
    if(cmd.substring(0,5) == "START")
    {
      mosturacao = true;
      liga_bomba;
      cont_tempo = 0;
    }
    if(cmd.substring(0,5) == "FERVR")
    {
      fervura = true;
      liga_bomba;
      cont_tempo = 0;
    }
  }
}

void ControlaResistencia()
{
static unsigned char tempo = 17;
static unsigned char cont_ciclos = 19;
  if(tempo)
  {
  tempo --;
  }
  else
  {
  tempo = 17;

    if(potencia > cont_ciclos)
    {
    liga_resistencia;      
    }
    else
    {
    desliga_resistencia;        
    }
  
    if(cont_ciclos)
    {
    cont_ciclos--; 
    }
    else
    {
    cont_ciclos = 19;   
    }
  }
}
