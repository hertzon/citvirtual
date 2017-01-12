#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0
#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin

#include <SPI.h>          // f.k. for Arduino-1.5.2
#include "Adafruit_GFX.h"// Hardware-specific library
#include <MCUFRIEND_kbv.h>
#include <Wtv020sd16p.h>
#include <SPI.h>
#include <SD.h>



MCUFRIEND_kbv tft;
File myFile;

const int chipSelect = 10;

// Assign human-readable names to some common 16-bit color values:
#define  BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF


int resetPin = 50;  // The pin number of the reset pin.
int clockPin = 53;  // The pin number of the clock pin.
int dataPin = 52;  // The pin number of the data pin.
int busyPin = 51;  // The pin number of the busy pin.

int hang=47;
int resetgsm=43;
int sensorColgar=42;

uint8_t aspect;
uint16_t pixel;
char *aspectname[] = {
  "PORTRAIT", "LANDSCAPE", "PORTRAIT_REV", "LANDSCAPE_REV"
};

char buffer[50];
int conteo=0;
int i=0;
String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete
int ri=24;
String value1, value2, value3, value4, value5;
String strCsq;
String strIncomingNumber;
char creg;
signed int csqval=0;
signed int rssi=0;
signed int rssiporc=0;
char prescaler=0;
int prescaler1=0;
boolean descolgo=false;
char cpas=0;
boolean colgado=true;
String voltajePorc="";
String voltajemV="";
String fila="";
char charRead;
char charBuf[50];
String numero="";
String bloque="";
String interior="";
String nombre="";

Wtv020sd16p wtv020sd16p(resetPin,clockPin,dataPin,busyPin);

void init_lcd(){
  uint16_t g_identifier;  
  g_identifier = tft.readID(); //
  Serial.print("ID = 0x");
  Serial.println(g_identifier, HEX);
  if (g_identifier == 0x00D3 || g_identifier == 0xD3D3) g_identifier = 0x9481; // write-only shield
  if (g_identifier == 0xFFFF) g_identifier = 0x9341; // serial //    g_identifier = 0x9329;                             // force ID
  tft.begin(g_identifier);

  tft.fillScreen(BLUE);
  tft.setRotation(45);
  tft.setTextSize(5);
  //drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color),
  //fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color),
  //tft.drawFastHLine(0, 45, 500,YELLOW);
  //drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color),
//  for (i=0;i<20;i++){
//    tft.drawRect(0+i, 0+i, 280, 230, YELLOW);  
//  }
  
  tft.fillRect(0, 0, 10, 320, YELLOW);
  tft.fillRect(470, 0, 10, 320, YELLOW);

  tft.fillRect(0, 0, 470, 10, YELLOW);
  tft.fillRect(0, 310, 470, 10, YELLOW);
  
  
}

void resetGsm(){
  Serial.println("Reseteando modulo gsm....");
  digitalWrite(resetgsm,HIGH);
  delay(200);
  digitalWrite(resetgsm,LOW);
}


void setup() {
  // put your setup code here, to run once:
  pinMode(resetgsm,OUTPUT);
  digitalWrite(resetgsm,LOW);
  pinMode(sensorColgar,INPUT);
  digitalWrite(sensorColgar,HIGH);      //Habilitamos pullup

  
  Serial.begin(115200);
  Serial1.begin(9600);
  pinMode(ri,INPUT);
  
  wtv020sd16p.reset();
  init_lcd();
  tft.fillRect(0, 45, 470, 5, YELLOW);
  tft.fillRect(0, 120, 470, 5, YELLOW);
  tft.fillRect(0, 195, 470, 5, YELLOW);

  tft.fillRect(240, 200, 5, 320, YELLOW);//Linea divisoria
  Serial1.flush();
  // reserve 200 bytes for the inputString:
  inputString.reserve(500);
  value1.reserve(20);value2.reserve(20);value3.reserve(20);value4.reserve(20);
  strCsq.reserve(5);strIncomingNumber.reserve(15);
  //Serial1.println("AT+CBAND=ALL_BAND");
 // delay(2000);
  Serial1.println("AT+CIURC=0");delay(300);
  //Serial1.println("AT+CFGRI=0");delay(300);
  //Serial1.println("AT&F0");delay(300);
  //Serial1.println("AT&W");delay(300);
  Serial1.println("AT+CLIP=1");delay(300);

  Serial.println("Initializing SD card...");
  pinMode(SS, OUTPUT);
  if (!SD.begin(chipSelect,11,12,13)) {
    Serial.println("initialization failed!");
    return;
  }
  myFile = SD.open("phones.txt", FILE_WRITE);
  if (myFile){
    Serial.println("txt file phones.txt open ok!");
  }else {
    Serial.println("error opening phones.txt!!");  
  }
  myFile.close();


  
  Serial.println("Sistema inicializado....");
  
}

void serial1flush(){
  while (Serial1.available()>0){
      char trash = (char)Serial1.read(); 
      delay(20);
  }  
}

void loop() {
  // put your main code here, to run repeatedly:
  serial1flush(); 
  //resetGsm();
//  while (1){
//    if (Serial1.available()>0){
//      Serial.print((char)Serial1.read());
//    }
//  }


  
  if (!digitalRead(ri)){
    
    if (++prescaler1>1000){
      prescaler1=0;
      leerCreg();//Leemos estado registro en red GSM
      delay(50);
      leerCsq();// Leemos calidad de la señal GSM
      delay(50);  
      leerStatus();// Leemos status del modem GSM
      delay(50);  
      leerVoltaje();
      Serial.println("************************************************************");
    }
    
    
    strIncomingNumber="";
  }else {
    //Serial.println("RI=1");
    if (creg=='1'){    
      Serial.println("Llamada entrante...");
      //wtv020sd16p.asyncPlayVoice(6);    
      delay(300);
      serial1flush(); 
      inputString="";
      while (digitalRead(ri)){
        //Cuando hay llamada entrante
        //Sacamos el numero telefonico entrante
        Serial.println("A");
        unsigned long timeout=0;
        boolean estadoColgado=digitalRead(hang);
        
        
        while (Serial1.available()==0 && ++timeout<300000){
          delayMicroseconds(10);   
//          if (digitalRead(hang)!=estadoColgado){
//            descolgo=true;    
//            goto enda;
//          }
          if (!digitalRead(sensorColgar)){
            Serial.println("Descolgo...");
            descolgo=true;
            goto enda;
          }
        }
        if (Serial1.available()>0){
          do {
          //Serial.println("lido:");
          readSerialString(1,false);  
          if (inputString=="\nNO CARRIER"){
            Serial.println("No carrier detectado...");
            goto enda;
          }
          
          }while (inputString!="\nRING");
          readSerialString(2,false);
          //Serial.println("InputString llamada entrante: "+inputString);
          for (int i = 0; i < inputString.length(); i++) {
            if (inputString.substring(i, i+1) == "\n") {
              value1 = inputString.substring(0, i);
              value2= inputString.substring(i+1);
              break;
            }
          }
          //Serial.println("Value1: "+value1);
          //Serial.println("Value2: "+value2);
          signed int index1=value2.indexOf('"');
          signed int index2=value2.indexOf('"',index1+1);
          //Serial.print("Index1: ");Serial.println(index1);  
          //Serial.print("Index2: ");Serial.println(index2);
          if (index1!=-1 && index2!=-1){           
            if ((index2-index1)>0){
              strIncomingNumber=value2.substring(index1+1,index2); 
              Serial.println("IncomingNumber: "+strIncomingNumber);
              wtv020sd16p.asyncPlayVoice(6);
              if (buscarNumero(strIncomingNumber)){
                Serial.println("Numero: "+strIncomingNumber+" encontrado");

                
              }
              
              //tft.reset();
              //init_lcd();         
            }
          }  
        }
        delay(20);
        serial1flush();  
        inputString="";
      }
      Serial.println("Saliendo de Llamada entrante...");
      enda:
      if (descolgo){
        Serial.println("Descolgo.....");
        descolgo=false;
        delay(200);
        Serial.println("Contestando.....");
        Serial1.println("ATA");
      }
      serial1flush(); 
      inputString="";
      wtv020sd16p.stopVoice();
      Serial.println("B");
    }else {
      if (Serial1.available()>0){
        Serial.print((char)Serial1.read());
      }
    }
  }
//  
//  if (++prescaler==5){
//    prescaler=0;
//    tft.setTextSize(4);
//    tft.setTextColor(GREEN, BLUE);
//    tft.setCursor(35, 13); 
//    tft.print("Citofonia Virtual");
//    tft.setTextSize(6);
//    tft.setCursor(20, 65); 
//    tft.setTextColor(GREEN, BLUE);
//    tft.print("Interior:");
//    sprintf(buffer, "%d",conteo);
//  
//    tft.setTextColor(WHITE, BLUE);
//    tft.print(buffer);
//    
//    tft.setCursor(20, 140); 
//    tft.setTextColor(GREEN, BLUE);
//    tft.print("Casa:");
//    tft.setTextColor(WHITE, BLUE);
//    tft.print("108");
//  
//    tft.setTextSize(4);
//    tft.setCursor(50, 210);
//    tft.setTextColor(GREEN, BLUE);
//    tft.print("Estado");
//  
//    tft.setTextSize(3);
//    tft.setCursor(25, 260);
//    tft.setTextColor(YELLOW, BLUE);
//    tft.print("Esperando..");
//  
//    tft.setTextSize(2);
//    tft.setCursor(270, 210);
//    tft.setTextColor(YELLOW, BLUE);
//    sprintf(buffer, "RSSI: %d %%",rssiporc);
//    tft.print(buffer);
//    
//    tft.setTextSize(2);
//    if (creg=='1'){      
//      tft.setCursor(270, 230);
//      tft.setTextColor(YELLOW, BLUE);
//      tft.print("RED: Ok");  
//    }else  if (creg=='2'){
//      tft.setCursor(270, 260);
//      tft.setTextColor(YELLOW, BLUE);
//      tft.print("RED: buscando...");  
//    }else if (creg=='3'){
//      tft.setCursor(270, 260);
//      tft.setTextColor(YELLOW, BLUE);
//      tft.print("RED: denegada");   
//    }else  if (creg=='4'){
//      tft.setCursor(270, 260);
//      tft.setTextColor(YELLOW, BLUE);
//      tft.print("RED: unknow");   
//    }else  if (creg=='5'){
//      tft.setCursor(270, 260);
//      tft.setTextColor(YELLOW, BLUE);
//      tft.print("RED: ok, roaming");   
//    }
//    conteo++;
//    if (conteo==99){
//      conteo=0;
//    }
//    
//  }
  delay(1);
}



boolean buscarNumero(String number){
  boolean encontrado=false;
  numero="";bloque="";interior="";nombre="";
   myFile = SD.open("phones.txt");
   if (myFile) {
    Serial.println("Leyendo: phones.txt "); 
    while (myFile.available()) {
      charRead=myFile.read();
      if (charRead!=0x0a){
         fila=charRead+fila;  
      }
      if (charRead==0x0a){
        fila.toCharArray(charBuf, 50);
        strrev(charBuf);
        fila=String(charBuf);
        Serial.println(fila);
        String value;       
        signed int index1=0;
        signed int index2=posicionChar(fila,',',1);
        if (index1!=-1 && index2!=-1 && (index2-index1)>0){
          numero=fila.substring(index1,index2);          
          if (numero==number){
            encontrado=true;
            Serial.println("Coincidencia encontrada");
          }
          Serial.println("Numero: "+numero);
        }
        index1=posicionChar(fila,',',1);
        index2=posicionChar(fila,',',2);
        if (index1!=-1 && index2!=-1 && (index2-index1)>0){
          bloque=fila.substring(index1+1,index2);
          Serial.println("Bloque: "+bloque);
        }
        index1=posicionChar(fila,',',2);
        index2=posicionChar(fila,',',3);
        if (index1!=-1 && index2!=-1 && (index2-index1)>0){
          interior=fila.substring(index1+1,index2);
          Serial.println("Interior: "+interior);
        }
        index1=posicionChar(fila,',',3);
        index2=fila.length();
        if (index1!=-1 && index2!=-1 && (index2-index1)>0){
          nombre=fila.substring(index1+1,index2);
          Serial.println("Nombre: "+nombre);
        }
        Serial.println("++++++++++++++++++++++++++++++++++++++++++++++++++++");
        fila="";   
        if (encontrado){
          Serial.println("Se encontro la siguiente coincidencia: ");
          Serial.println("Numero: "+numero);Serial.println("Nombre: "+nombre);Serial.println("Bloque: "+bloque);Serial.println("Interior: "+interior);
          break;
        }
      }    
    }
    Serial.println("Termino lectura de memoria SD");
    // close the file:
    myFile.close();
   }else {
    // if the file didn't open, print an error:
    Serial.println("error opening phones.txt");
  }



  return encontrado;
}

void readSerialString(int n,boolean show){
  char inChar;
  unsigned int timeoutSerial=0;
  boolean errorSerial=false;
  unsigned int timeout=0;
  inputString="";
  //Serial.println("a1");
  for (int ntimes=0; ntimes<n;ntimes++){
    do {
      timeoutSerial=0;
      while (Serial1.available()==0 && ++timeoutSerial<50000){//0.5 seconds
        //Esperamos a que llegue primer byte  
        delayMicroseconds(20);
        
      }
      if (Serial1.available()==0){
        errorSerial=true;
        Serial.println("No hay data serial");
        goto endSerial;
      }
      inChar = (char)Serial1.read();  
      if (inChar!='\r'){
        inputString += inChar;  
      }
    }while (inChar!='\r');
      
  }
  //Serial.println("a2");
  if (show){
    Serial.println("LeidoString: "+inputString);  
  }
  //Serial.println("a3");
  endSerial:
  delayMicroseconds(100);
}

void leerCsq(){
  serial1flush();  
  inputString = ""; 
  value1="";value2="";value3="";value4=""; 
  Serial1.println("AT+CSQ");
  readSerialString(3,false);
  
  
//  delay(300);
//  while (Serial1.available()>0){
//    char inChar = (char)Serial1.read();  
//    if (inChar!='\r'){
//      inputString += inChar;  
//    }
//  } 
  //Serial.println("InputString: "+inputString);
  for (int i = 0; i < inputString.length(); i++) {
    if (inputString.substring(i, i+1) == "\n") {
      value1 = inputString.substring(0, i);
      value2= inputString.substring(i+1);
      break;
    }
  }
  //Serial.println("Value2: "+value2);
  signed int index1=value2.indexOf(' ');
  signed int index2=value2.indexOf(',');
  if (index1!=-1 && index2!=-1){
    //Serial.print("Index1: ");Serial.println(index1);  
    //Serial.print("Index2: ");Serial.println(index2); 
    if ((index2-index1)>0){
      strCsq=value2.substring(index1,index2);        
      csqval=strCsq.toInt();
      if (csqval<1){
        rssi=-115;  
      }else if (csqval==1){
        rssi=-111;  
      }else if (csqval>30){
        rssi=-52;  
        
      }else{
        rssi=2*csqval-114;
      }
      rssiporc=(int)(1.6667*rssi+185);
      Serial.print("CSQ: ");Serial.print(csqval); 
      Serial.print(" RSSI: ");Serial.print(rssi);Serial.print("dBm");
      Serial.print(" RSSI[%]: ");Serial.print(rssiporc);
      Serial.println("");
    }
  }
}

signed int posicionChar(String x,char c,int pos){
  int i=0;
  signed int ret=-1;
  int offset=0;
  for (i=0;i<pos;i++){
    ret=x.indexOf(c,offset); 
    offset=ret+1;
  }
  return ret;
}

void leerVoltaje(){
  serial1flush();
  inputString = ""; 
  value1="";value2="";value3="";value4=""; 
  Serial1.println("AT+CBC");
  readSerialString(3,false);
  for (int i = 0; i < inputString.length(); i++) {
    if (inputString.substring(i, i+1) == "\n") {
      value1 = inputString.substring(0, i);
      value2= inputString.substring(i+1);
      break;
    }
  }
  //Serial.println("value2: "+value2);
  signed int index1=posicionChar(value2,',',1);
  signed int index2=posicionChar(value2,',',2);
  if (index1!=-1 && index2!=-1 && (index2-index1)>0){
    voltajePorc=value2.substring(index1+1,index2);
    voltajemV=value2.substring(index2+1,value2.length());
    Serial.print("Voltaje %: ");Serial.print(voltajePorc);Serial.print("% ");Serial.print("Voltaje mV: ");Serial.println(voltajemV);
    
  }
}


void leerStatus(){
  serial1flush();
  inputString = ""; 
  value1="";value2="";value3="";value4="";
  Serial1.println("AT+CPAS");    
  readSerialString(3,false);
  for (int i = 0; i < inputString.length(); i++) {
    if (inputString.substring(i, i+1) == "\n") {
      value1 = inputString.substring(0, i);
      value2= inputString.substring(i+1);
      break;
    }
  }
  //Serial.println("value1: "+value1); 
  //Serial.println("value2: "+value2); 
  cpas=(char)value2.charAt(7);
  Serial.print("CPAS: ");
  Serial.print(cpas); Serial.print(" ");
  if (cpas=='0'){
    Serial.println("Ready");
  }else if (cpas=='2'){
    Serial.println("Unknow");   
  }else if (cpas=='3'){
    Serial.println("Ringing");   
  }else if (cpas=='4'){
    Serial.println("Call in progress");   
  }
}

void leerCreg(){
  serial1flush();
  inputString = ""; 
  value1="";value2="";value3="";value4="";
  Serial1.println("AT+CREG?");
  readSerialString(3,false);
  
  //Serial.println("Available bytes: ");
  //Serial.println(Serial1.available());
//  while (Serial1.available()>0){
//    char inChar = (char)Serial1.read();  
//    if (inChar!='\r'){
//      inputString += inChar;  
//    }
//    
//  }
  //Serial.println("InputString: "+inputString);  
  for (int i = 0; i < inputString.length(); i++) {
    if (inputString.substring(i, i+1) == "\n") {
      value1 = inputString.substring(0, i);
      value2= inputString.substring(i+1);
      break;
    }
  }
  //Serial.println("value1: "+value1); 
  //Serial.println("value2: "+value2); 
//  for (int i = 0; i < value2.length(); i++) {
//    if (value2.substring(i, i+1) == "\n") {
//      value3 = value2.substring(0, i);
//      break;
//    }
//  }
//  for (int i = 0; i < value2.length(); i++) {
//    if (value2.substring(i, i+1) == ",") {
//      value4 = value2.substring(i+1);        
//      break;
//    }
//  }
  creg=(char)value2.charAt(9);
  Serial.print("CREG: ");
  Serial.println(creg);
  
}

/*
  SerialEvent occurs whenever a new data comes in the
 hardware serial RX.  This routine is run between each
 time loop() runs, so using delay inside loop can delay
 response.  Multiple bytes of data may be available.
 */
//void serialEvent() {
//  while (Serial1.available()) {
//    // get the new byte:
//    char inChar = (char)Serial1.read();
//    // add it to the inputString:
//    inputString += inChar;
//    // if the incoming character is a newline, set a flag
//    // so the main loop can do something about it:
//    if (inChar == '\n') {
//      stringComplete = true;
//    }
//  }
//}

char strccnt(char *strptr, char c,char n){
    // Find the number of characters 'c' in the target string
        char count = 0;
        char temp;
        char i=0;
        temp = *strptr;         // get the first character of the string

        while(temp != 0x00){
            // continue to the null terminator
          if(temp == c){         // if the character is equal to search character
              count++;            // increment the character counter
              if (count==n){
                 break;
              }
          }
        temp = *strptr++;      // move to next charcter in the string
        i++;
   }
   //return(count);
   return(i);
}

