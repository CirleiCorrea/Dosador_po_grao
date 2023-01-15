//=====================Bibliotecas========================
#include <LiquidCrystal.h>
#include <HX711.h>


//=================Constantes e define========================
#define BT_up       A0
#define BT_down     A1
#define BT_select   A2
#define Saida       10
#define DataPin     8
#define ClockPin    7
#define minPWM      60.0
#define maxPWM      115.0
#define rs 12
#define en 11
#define d4 5
#define d5 4
#define d6 3
#define d7 2

//=================Variaveis globais========================
float  PesoAtual   = 0.0,
       PesoAjuste  = 0.0,
       PWM_control = 0.0;
      
      
int opc  = 0;

int Porcentagem = 20;
    
    
boolean Ativar  = true,
        bt1     = false,
        bt2     = false,
        bt3     = false;



//=================Objetos========================
HX711 scale;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);



//=================Prototipo das funções========================

int LerBotao();
boolean BTselect();
float ControlScale(float PesoDesejado, float Pote, float Porcento);
float AutoOffset(float Erro, byte Divisor);

void ZerarBlc();
void Automatico();
void AjustePeso();
void AjustePercentual();

void setup(){
  scale.begin(DataPin, ClockPin);
  lcd.begin(16, 2);
  Serial.begin(9600);

  scale.set_scale(103);
  scale.tare();

  pinMode(BT_up, INPUT_PULLUP);
  pinMode(BT_down, INPUT_PULLUP);
  pinMode(BT_select, INPUT_PULLUP);
  pinMode(Saida, OUTPUT);
}

void loop(){
  
  opc += LerBotao();
  if(opc < 0) opc = 4;
  if(opc > 4) opc = 1;

  switch (opc) {
    case 1:
      lcd.setCursor(0,0);
      lcd.print("-> Automatico    ");
      lcd.setCursor(0,1);
      lcd.print("Ajustar o peso   ");
  
       if (BTselect()){
         if(PesoAjuste <= 0){
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Erro! Ajuste o");
          lcd.setCursor(0,1);
          lcd.print("peso primeiro");
          delay(1000);
          AjustePeso();
         }else{
          lcd.clear();   
          Automatico();
         }
        }
      break;
    case 2:
      lcd.setCursor(0,0);
      lcd.print("->Ajustar o peso   ");
      lcd.setCursor(0,1);
      lcd.print("Zerar balanca    ");
      
       if (BTselect())
       {
        Ativar = true;     
        AjustePeso();  
        }
      break;
    case 3:
      lcd.setCursor(0,0);
      lcd.print("-> Zerar balanca    ");
      lcd.setCursor(0,1);
      lcd.print("Percentual      ");
       if (BTselect())
       {
        lcd.clear();
        Ativar = true;
        ZerarBlc();
        }
      break;
    case 4:
      lcd.setCursor(0,0);
      lcd.print("-> Percentual     ");
      lcd.setCursor(0,1);
      lcd.print("Automatico      ");
  
      if (BTselect())
      {
        Ativar = true;   
        lcd.clear();
        AjustePercentual();
      }
      break;
    default:
      lcd.setCursor(0, 0);
      lcd.print("Tecle UP ou DOWN");
      lcd.setCursor(0, 1);
      lcd.print("Select p/ entrar");
      break;
  }
}
int LerBotao(){
  if (!digitalRead(BT_up)) bt1 = !bt1;
    if (digitalRead(BT_up) && bt1) { 
      bt1 = !bt1;
      return 1;      
      }
  if (!digitalRead(BT_down)) bt2 = !bt2;
    if (digitalRead(BT_down) && bt2) {      
      bt2 = !bt2; 
      return -1;
    }
    
  return 0;
}  //end ler_bt

boolean BTselect(){
  if (!digitalRead(BT_select)) bt3 = !bt3;
    if (digitalRead(BT_select) && bt3) {
      bt3 = !bt3;
      return 1;
    }
  return 0;
  
}//end BTselect()

void Automatico() {
  
  boolean pAjuste = false;

  float RangerPeso = 0.0;
          
  static boolean ativeTR = false;
          
          
  static float pPote         = 0.0,
               porcento      = 0.0,
               PoteAtulizado = 0.0;
          
  lcd.clear();

  if(ativeTR){
    lcd.setCursor(0, 0);
    lcd.print("Zere a balanca");
    lcd.setCursor(0, 1);
    lcd.print("primeiro s/ pote");
    delay(1300);
    ativeTR = !ativeTR;
    Ativar = true;
    ZerarBlc(); 
  }

   do{
 
      lcd.setCursor(0, 0);
      lcd.print("Coloque o pote!");
      lcd.setCursor(0, 1);
      lcd.print("Pressione select");
      
      if (BTselect()){
        lcd.clear();
        scale.power_up();
        pPote = scale.get_units(6);
        Serial.print("pPote antes: ");
        
        Serial.println(pPote); 
        porcento = pPote*0.1;
        PoteAtulizado = pPote - porcento;
        Serial.print("porcento: ");
        Serial.println(porcento);
        Serial.print("PoteAtulizado: ");
        Serial.println(PoteAtulizado);
        
        lcd.setCursor(0, 0);
        lcd.print("Valor salvo!");
        delay(1000);
        Ativar = true;  
      }
    }while(!Ativar);

    while(!BTselect()){
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Peso: ");
      lcd.setCursor(8, 0);
      lcd.print(PesoAjuste);
      lcd.setCursor(0, 1);
      lcd.print("Atual: ");
      lcd.setCursor(8, 1);
      lcd.print(PesoAtual - pPote);
      PesoAtual = scale.get_units(4);

      RangerPeso = (PesoAjuste/100)*Porcentagem;

      if(PesoAtual <= PoteAtulizado && PesoAtual > 0) pAjuste = 1;
      if(PesoAtual >= PoteAtulizado && pAjuste)
      {
        pAjuste = 0;
        ControlScale(PesoAjuste, pPote, RangerPeso);
      }
     }
     lcd.clear();
   } //end automatico
   
float ControlScale(float PesoDesejado, float Pote, float Porcento){
         float Diferenca = 0.0;
  
  static float Offset    = 0.0;
        
  byte  PWM       = 0;

          
   for(PesoAtual = 0; PesoAtual <= PesoDesejado - Offset;){    
    Serial.print("Peso: ");
    Serial.println(PesoAtual);
    Serial.print("diferenca: ");
    Serial.println(Diferenca);
    Serial.print("Offset: ");
    Serial.println(Offset);

    PesoAtual = scale.get_units(3)-Pote;
    
    Diferenca = PesoDesejado - PesoAtual;

    lcd.setCursor(8, 1);
    lcd.print(PesoAtual);

    if(Porcento < Diferenca){
      analogWrite(Saida, 255);
    }else{
      PWM = map(Diferenca, 0, 100, minPWM, maxPWM);
      analogWrite(Saida, PWM);
    }
   }
   
   Offset = AutoOffset(Diferenca, 4);
    
   analogWrite(Saida, LOW);

   lcd.clear();
   lcd.print("Peso ok");
   delay(1300);
  
}//end ControlScale

float AutoOffset(float Erro, byte Divisor){
  static byte   Count = 0;
  
  static float  SomaErro  = 0.0,
                NewOffset = 0.0;
                
  SomaErro += Erro;
  Count++;
  
  if(Count == Divisor){
    NewOffset = SomaErro/Divisor;
    SomaErro = 0;
    Count = 0;
  }

  Serial.print("NewOffset: ");
   Serial.println(NewOffset);
  return NewOffset;
}

void AjustePeso() {
  
  lcd.clear();
  
  while(!BTselect()){
      
      lcd.setCursor(0, 0);
      lcd.print("Peso: ");
      lcd.setCursor(0, 1);
      lcd.print(PesoAjuste);
      lcd.setCursor(5, 1);
      lcd.print("g");
      
      PesoAjuste += (float)LerBotao()/2;

      if(PesoAjuste < 0){
        PesoAjuste = 0;
        lcd.clear();
        lcd.print("Peso menor que 0");
        delay(2000);
        lcd.clear();
      }
      if(PesoAjuste > 1000.0)
      {
        PesoAjuste = 1000.0;
        lcd.clear();
        lcd.print("Peso no limite");
        delay(2000);
        lcd.clear();
      }
     }
     lcd.clear();
}//end AjustePeso()

void ZerarBlc() {

  lcd.clear();

  scale.power_up();
  
    while(Ativar){
      
      lcd.setCursor(0,0);
      lcd.print("Peso: ");
      lcd.setCursor(7,0);
      lcd.print(scale.get_units(5));
      
      if (BTselect()){
        lcd.clear();
        lcd.setCursor(4,0);
        lcd.print("Tara ok!");
        scale.tare();
        scale.power_down();
        Ativar = false;
      }
   }
  lcd.clear();
}//end tara_peso()

void AjustePercentual(){

  lcd.clear();
  
  while(Ativar){
    lcd.setCursor(0,0);
    lcd.print("% de Corte");
    lcd.setCursor(0, 1);
    lcd.print(Porcentagem);

    Porcentagem += LerBotao();

    if (BTselect()){
      lcd.clear();
      Ativar = false;
      }
  }
}//end AjustePercentual()








  
