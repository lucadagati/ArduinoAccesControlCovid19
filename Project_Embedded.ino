//INCLUDO LIBRERIA RFID
#include <MFRC522.h>
//INCLUDO LIBRERIA EEPROM 
#include <EEPROM.h>
//INCLUDO LIBRERIA PER LA GESTIONE LCD TRAMITE REGISTRO
#include <Wire.h>
#include <LiquidCrystal_SR3W.h>
//INCLUDO LIBRERIA KEYPAD
#include <Keypad.h>
//INCLUDO LIBRERIA SERVO MOTORE
#include <Servo.h>
//INCLUDO LIBRERIA SENSORE ULTRASONICO
#include "SR04.h"
//INCLUDO LIBRERIA SENSORE TEMPERATURA
#include <dht_nonblocking.h>

//RFID
MFRC522 rfid(10, 9); //COMUNICAZIONE CON RFID TRAMITE PIN 9 E 10


//EEPROM
//Constanti per la gestione della memoria EEPROM, minimo e massimo degli indirizzi EEPROM
const int EEPROM_MIN_ADDR = 0;
const int EEPROM_MAX_ADDR = 511; //Possiamo utilizzarli fino all’indirizzo 1023

//LIQUID CRYSTAL DISPLAY
//Costranti per la gestione dello shift register per utilizzare l'LCD
const int PIN_LCD_STROBE =  7; // Out: LCD IC4094 shift-register strobe LACH PIN  - STCP
const int PIN_LCD_DATA =  6; // Out: LCD IC4094 shift-register data     DATA PIN  - DS
const int PIN_LCD_CLOCK =  5; // Out: LCD IC4094 shift-register clock   CLOCK PIN - SHCP
// srdata / srclock / strobe / bl pin on SR / blpol (positive\negative)
LiquidCrystal_SR3W lcd(PIN_LCD_DATA, PIN_LCD_CLOCK, PIN_LCD_STROBE, 7, POSITIVE); // 7 e POSITIVE definiscono lo stato associato all'output 7 

//KEYPAD
const byte ROWS = 3; //3 righe (ho scollegato la quarta riga)
const byte COLS = 3; //3 colonne (ho scollegato la quarta colonna)
char hexaKeys[ROWS][COLS] = { //definisco la matrice bidimensionale dei pulsanti del keypad
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
   };
byte rowPins[ROWS] = {19, 18, 17}; //utilizzo gli ingressi A3 A4 A5 come ingressi digitali che hanno numerazione 17 18 e 19 per gli ingressi righe
byte colPins[COLS] = {16, 15, 14}; //utilizzo gli ingressi A1 A2 come ingressi digitali che hanno numerazione 15 e 16, ed l'ingresso digitale 8 per gli ingressi a colonna
Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);  //Inizializza un'istanza della classe NewKeypad
const char PIN[] ={'1','2','3','4'};

//SERVOMOTORE
Servo myservo; //Creo un'istanza della classe Servo


//SENSORE ULTRASONICO
//definisco i pin ai quali il sensore è collegato
#define TRIG_PIN 4
#define ECHO_PIN 2
SR04 sr04 = SR04(ECHO_PIN,TRIG_PIN); //creo un'istanza per il sensore

//SENSORE TEMPERATURA
#define DHT_SENSOR_TYPE DHT_TYPE_11
static const int DHT_SENSOR_PIN = 8;
DHT_nonblocking dht_sensor( DHT_SENSOR_PIN, DHT_SENSOR_TYPE );


void setup() {

//FLASHING EEPROM
  /*
  lcd.clear();
  lcd.home (); 
  lcd.setCursor (0, 0); //XY
  lcd.print(F("FLASHING EEPROM"));
  delay(2000);
  for (int i = 0 ; i < EEPROM.length() ; i++) {EEPROM.write(i, 0);}
  lcd.clear();
  lcd.home (); 
  lcd.setCursor (0, 0); //XY
  lcd.print(F("FLASHED EEPROM"));
  delay(2000);
  */

//  printUID(); //Verifica di tutti gli id refid presenti nella eeprom
  
  //INIZIALIZZAZIONE MOTORE SERVO
  myservo.attach(3);//connect pin 9 with the control line(the middle line of Servo) 
  myservo.write(0);// lo pongo in posizione zero

  lcd.begin(16, 2);  // Inizializzo l'LCD per 16 char su 2 linee
  
  //Gli ingressi dal 15 al 19 vanno utilizzati come input
  pinMode(14,INPUT);
  pinMode(15,INPUT);
  pinMode(16,INPUT);
  pinMode(17,INPUT);
  pinMode(18,INPUT);
  pinMode(19,INPUT);

  //Inizializzazione RFID
  SPI.begin();            // Init SPI bus
  rfid.PCD_Init();

  //Monitor Seriale
  Serial.begin(9600);
}
  


void loop() {
  
  lcd.clear();
  lcd.home (); 
  lcd.setCursor (0, 0); //XY
  lcd.print(F("RDIF ACCESS: 1"));
  lcd.setCursor (0, 1);
  lcd.print(F("PIN  ACCESS: 2"));

  char customKey;
  
//ATTENDO L'INSERIMENTO DA KEYPAD
  while(true){
      customKey = customKeypad.getKey();
        if (customKey=='1' || customKey=='2')
          break;         
   }
 
  switch(customKey){
    case '1':
            lcd.clear();
            lcd.home (); 
            lcd.setCursor (0, 0); //XY
            lcd.print(F("Avvicinare RFID"));
            delay(5000);
            if(CheckRFIDuid()){
               lcd.clear();
               lcd.home (); 
               lcd.setCursor (0, 0); //XY
               lcd.print(F("RFID"));
               lcd.setCursor (0, 1); //XY
               lcd.print(F("ABILITATO"));
               delay(3000);
            }
           else{
               lcd.clear();
               lcd.home (); 
               lcd.setCursor (0, 0); //XY
               lcd.print(F("RFID"));
               lcd.setCursor (0, 1); //XY
               lcd.print(F("NON ABILITATO"));
               delay(3000);
               break;
           }
           getClose();
           if(getemperature()){
              lcd.clear();
              lcd.home (); 
              lcd.setCursor (0, 0); //XY
              lcd.print(F("PREGO, PUO'"));
              lcd.setCursor (0, 1); //XY
              lcd.print(F("ACCEDERE"));
              delay(3000);
              servomotor();
            }
           else{
              lcd.clear();
              lcd.home (); 
              lcd.setCursor (0, 0); //XY
              lcd.print(F("RILEVATA FREBBRE"));
              lcd.setCursor (0, 1); //XY
              lcd.print(F("VIETATO ENTRARE"));
              delay(3000);
           }
            break;

    case '2':
      lcd.clear();
      lcd.home (); 
      lcd.setCursor (0, 0); //XY
      lcd.print(F("Inserire PIN"));  //IMPOSTIAMO PASSWORD "1234"

      //inserimento pin
      char pin_key[4];
      for(int j=0;;){
        customKey = customKeypad.getKey();
        if (customKey){
          pin_key[j]=customKey;
          lcd.setCursor (j, 1); //XY
          lcd.print(F("*"));
          j++;
          if (j==4)
            break;
        }
      }
      delay(1000);

      //SE IL CODICE CORRISPONDE AL PC CONSENTI L'ACCESSO
      if(pin_key[0]==PIN[0]&&pin_key[1]==PIN[1]&&pin_key[2]==PIN[2]&&pin_key[3]==PIN[3]){ //CONFRONTO OGNI SINGOLO CARATTERE
        lcd.clear();
        lcd.home (); 
        lcd.setCursor (0, 0); //XY
        lcd.print(F("ACCESSO"));
        lcd.setCursor (0, 1); //XY
        lcd.print(F("CONSENTITO"));
        delay(3000);
      }
      else {
        lcd.clear();
        lcd.home (); 
        lcd.setCursor (0, 0); //XY
        lcd.print(F("PIN ERRATO"));
        delay(3000);
        break;
      }

      lcd.clear();
      lcd.home (); 
      lcd.setCursor (0, 0); //XY
      lcd.print(F("VUOI REGISTRARE"));
      lcd.setCursor (0, 1);
      lcd.print(F("RFID? 1 SI 2 NO"));
      
      //ATTENDO L'INSERIMENTO DA KEYPAD
      while(true){
      customKey = customKeypad.getKey();
        if (customKey=='1' || customKey=='2')
          break;
      }
          
      switch (customKey){
         case '1':
            lcd.clear();
            lcd.home (); 
            lcd.setCursor (0, 0); //XY
            lcd.print(F("Avvicinare RFID"));
            delay(3000);

            if(SaveRFIDuid()){
               lcd.clear();
               lcd.home (); 
               lcd.setCursor (0, 0); //XY
               lcd.print(F("OPERAZIONE"));
               lcd.setCursor (0, 1); //XY
               lcd.print(F("COMPLETATA"));
            }
            else{
               lcd.clear();
               lcd.home (); 
               lcd.setCursor (0, 0); //XY
               lcd.print(F("ERRORE DURANTE"));
               lcd.setCursor (0, 1); //XY
               lcd.print(F("LA MEMORIZZAZIONE"));
            }
            delay(3000); 
            getClose();
            
           if(getemperature( )){
              lcd.clear();
              lcd.home (); 
              lcd.setCursor (0, 0); //XY
              lcd.print(F("PREGO, PUO'"));
              lcd.setCursor (0, 1); //XY
              lcd.print(F("ACCEDERE"));
              delay(3000);
              servomotor();
            }
           else{
              lcd.clear();
              lcd.home (); 
              lcd.setCursor (0, 0); //XY
              lcd.print(F("RILEVATA FREBBRE"));
              lcd.setCursor (0, 1); //XY
              lcd.print(F("VIETATO ENTRARE"));
              delay(3000);
           }
            break;

         case '2':
            getClose();
              if(getemperature()){
                 lcd.clear();
                 lcd.home (); 
                 lcd.setCursor (0, 0); //XY
                 lcd.print(F("PREGO, PUO'"));
                 lcd.setCursor (0, 1); //XY
                 lcd.print(F("ACCEDERE"));
                 delay(3000);
                 servomotor();
              }
           else{
              lcd.clear();
              lcd.home (); 
              lcd.setCursor (0, 0); //XY
              lcd.print(F("RILEVATA FREBBRE"));
              lcd.setCursor (0, 1); //XY
              lcd.print(F("VIETATO ENTRARE"));
              delay(3000);
           }
             break; 
            
            }
         default: 
           break;
   }
}





//FUNZIONI RFID

boolean CheckRFIDuid(){
  const int BUFSIZE = 9;
  char buf[BUFSIZE];
  String myString; 
  char myStringChar[BUFSIZE];

  String uid;
  //codice rfid
  if(rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()){
      uid = getUID();

      for(int a=1; a<=EEPROM.read(0)*9+1;a=a+9){ //la EEPROM può salvare al massimo 1023 byte, quindi essendo 9byte necessari per ogni UID, può salvare 113 Codici RFID
           eeprom_read_string(a, buf, BUFSIZE);
        if (uid==buf)
              return true;
      }
  }
  else {
    lcd.clear();
    lcd.home (); 
    lcd.setCursor (0, 0); //XY
    lcd.print(F("ERRORE DI LETTURA"));
    delay(2000);
    return false; //return false se fallisce la lettura dell'RFID
  }
    return false; //return false se l'ID non è presente in memoria
}

//salva l'ID dell'RFID nella EEPROM
boolean SaveRFIDuid(){
  const int BUFSIZE = 9;
  char buf[BUFSIZE];
  String myString; 
  char myStringChar[BUFSIZE];

  String uid;
  //codice rfid
  if(rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()){
      uid = getUID();
      for(int a=1; a<EEPROM.read(0)*9+1;a=a+9){ //la EEPROM può salvare al massimo 1023 byte, quindi essendo 9byte necessari per ogni UID, può salvare 113 Codici RFID
         eeprom_read_string(a, buf, BUFSIZE);
         if (uid==buf){
           lcd.clear();
           lcd.home (); 
           lcd.setCursor (0, 0); //XY
           lcd.print(F("RFID GIA'"));
           lcd.setCursor (0, 1); //XY
           lcd.print(F("ABILITATO"));
           delay(3000);
           return true;
        }
      }
      myString=uid;
      myString.toCharArray(myStringChar, BUFSIZE); //converto string in char array
      strcpy(buf, myStringChar);
      eeprom_write_string(EEPROM.read(0)*9+1, buf); 
      EEPROM.write(0, EEPROM.read(0)+1); //il primo byte della eeprom è dedicato ad un contatore, che tiene conto del numero di uid salvati ogni volta che ne viene aggiunto uno
      return true; 
      }
      else 
        return false;
}


//verifico se l’indirizzo della EEPROM è valido
boolean eeprom_is_addr_ok(int addr) {
  return ((addr >= EEPROM_MIN_ADDR) && (addr <= EEPROM_MAX_ADDR));
}


//scrive una sequenza di byte sulla eeprom iniziando da uno specifico indirizzo. 
boolean eeprom_write_bytes(int startAddr, const byte* array, int numBytes) {
  int i;
  if (!eeprom_is_addr_ok(startAddr) || !eeprom_is_addr_ok(startAddr + numBytes)) { //Ritorna false, e non scrive nulla,  se l'indirizzo di inizio o fine non sono tra gli indirizzi consentiti
    return false;
   }
  for (i = 0; i < numBytes; i++) {
    EEPROM.write(startAddr + i, array[i]);
  }
  return true;
}

//Scrive una stringa iniziando da un indirizzo specifico.
boolean eeprom_write_string(int addr, const char* string) {
  int numBytes; // numero di byte da scrivere
  //scrivo il contenuto della stinga più il byte terminator (0x00)
  numBytes = strlen(string) + 1;
  return eeprom_write_bytes(addr, (const byte*)string, numBytes); //ritorna true se la stringa è stata scritta correttamente, altrimenti se qualche bit ricade fuori range ritorna false
}

//Legge una stringa a partire da uno specifico indirizzo, ritorna true se almeno un byte è letto, ritorna false se l’indirizzo ricade al di fuori di quelli //consentiti. La lettura puo’fermarsi se: non vi è spazio nel buffer, l’ultimo indirizzo eeprom viene raggiunto, o incontra la stringa terminatore 0x00
boolean eeprom_read_string(int addr, char* buffer, int bufSize) {
    byte ch; // byte letti dalla eeprom
    int bytesRead; // numbero di byte letti 
    if (!eeprom_is_addr_ok(addr)) {         //controllo se l’indirizzo è valido 
    return false;
    }
  if (bufSize == 0) {     //esco se il buffer è vuoto
    return false;
  }
  if (bufSize == 1) {    // se c’è solo il carattere terminatore, esco
    buffer[0] = 0;
    return true;
  }
  bytesRead = 0;   // inizializzo il contatore per i bytes
  ch = EEPROM.read(addr + bytesRead);    // leggo il byte successivo dalla eeprom
  buffer[bytesRead] = ch;     // lo memorizzo nel buffer utente
  bytesRead++;     // incremento il contatore byte
  // Il while si ferma quando: Incontro il carattere 0x00, oppure ho riempito il buffer oppure ho raggiunto l’ultimo indirizzo eeprom
  while ( (ch != 0x00) && (bytesRead < bufSize) && ((addr + bytesRead) <= EEPROM_MAX_ADDR) ) {
    // finche’sono all’interno del while, leggo il byte successivo dalla eeprom
    ch = EEPROM.read(addr + bytesRead);
    buffer[bytesRead] = ch; // lo memorizzo nel buffer
    bytesRead++; // incremento il contatore
  }
  // mi assicuro che il buffer ha la stringa terminatore come ultimo byte, (0x00)
  if ((ch != 0x00) && (bytesRead >= 1)) {
    buffer[bytesRead - 1] = 0;
  }
  return true;
}


//FUNZIONE PER LA LETTURA DEL CODICE ID DELL'RFID
String getUID(){
  String uid = "";
  for(int i = 0; i < rfid.uid.size; i++){
    uid += rfid.uid.uidByte[i] < 0x9 ? "0" : "";  //Nel sistema HEX, lo zero iniziale è presente perché i numeri devono iniziare con un carattere numerico, e la 'x' significa esadecimale
    uid += String(rfid.uid.uidByte[i], HEX);
  }
  rfid.PICC_HaltA(); //Halt PICC
  return uid;
}

//FUNZIONE VICINANZA
boolean getClose(){
        while (sr04.Distance()>6){
           lcd.clear();
           lcd.home (); 
           lcd.setCursor (0, 0); //XY
           lcd.print(F("AVVICINARSI A 6"));
           lcd.setCursor (0, 1); //XY
           lcd.print(F("CM DAL SENSORE"));
           delay(1000);
           lcd.clear();
           lcd.home (); 
           lcd.setCursor (0, 0); //XY
           lcd.print(F("SEI A "));
           lcd.setCursor (7, 0); //XY
           lcd.print(sr04.Distance());
           lcd.setCursor (0, 1); //XY
           lcd.print(F("CM DAL SENSORE"));
           delay(1000);
        }
           lcd.clear();
           lcd.home (); 
           lcd.setCursor (0, 0); //XY
           lcd.print(F("CONTROLLO"));
           lcd.setCursor (0, 1); //XY
           lcd.print(F("IN CORSO"));
           delay(3000);
           return true;
}


static bool getemperature (){
    float temperature;
    float humidity;
    
    while(dht_sensor.measure(&temperature,&humidity)==false){}
    Serial.print( temperature );
    lcd.clear();
    lcd.home (); 
    lcd.setCursor (0, 0); //XY
    lcd.print(F("TEMPERATURA"));
    lcd.setCursor (0, 1); //XY
    lcd.print(temperature);/*stampiamo sul monitor la temperatura*/
    delay(1000);/*ritardo di un secondo*/
   if (temperature<37.5)
     return true; 
   else
     return false;   
}

void servomotor(){
  myservo.write(90);
  delay(5000);
  myservo.write(0);
}

  
//FUNZIONE DI CONTROLLO PER STAMPARE TUTTI GLI UID PRESENTI NELLA EEPROM 
/*
 void printUID(){
  const int BUFSIZE = 9;
  char buf[BUFSIZE];

  String uid;
  //codice rfid
      for(int a=1; a<=EEPROM.read(0)*9+1;a=a+9){ //la EEPROM può salvare al massimo 1023 byte, quindi essendo 9byte necessari per ogni UID, può salvare 113 Codici RFID
         eeprom_read_string(a, buf, BUFSIZE);
           lcd.clear();
           lcd.home (); 
           lcd.setCursor (0, 0); //XY
           lcd.print(buf);
           delay(1000);
      }
 }
 */
