
/*

        DCF77 Stationsuhr und NCDXF Bakenanzeige 
        by OE3GOD, Oct. 2016
        
        uses:
        Arduino UNO
        DCF77 Receiver by Conrad.at
        RTC DS1307 by SEMAF Electronics
        LCD I2C Display by SEMAF Electronics
        Nov. 2016:  DCF77 Updateintervall auf 900 Sek
                    Save Time sofort nach erstem Sync mit DCF77

*/

#include "DCF77.h"
#include <DS1307RTC.h>
#include <Time.h>
#include <TimeLib.h>
#include <LiquidCrystal_I2C.h>


#define DCF_PIN 2                // Connection pin to DCF 77 device
#define BAND_PIN 3               // Connection pin to Band Selector
#define DCF_INTERRUPT 0           // Interrupt number associated with pin
#define BAND_INTERRUPT 1          // Interrupt number associated with pin

LiquidCrystal_I2C lcd(0x27, 16, 2);

// inverted input on pin DCF_PIN
DCF77 DCF = DCF77(DCF_PIN,DCF_INTERRUPT, false); //PIN4 am DCF77 Modul

time_t prevDisplay = 0;
time_t time;
// Variables

int reading;
int prevhour=0;
int previous;
byte band, sekm,minm,feld,spalte,i,i2;
unsigned long alteZeit=0, entprellZeit=20;

byte  matrix[19][18] = { 
{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
{ 1,2,3,4,5,0,0,0,0,0,0,0,0,0,0,0,0,0},
{ 0,1,2,3,4,5,0,0,0,0,0,0,0,0,0,0,0,0},
{ 0,0,1,2,3,4,5,0,0,0,0,0,0,0,0,0,0,0},
{ 0,0,0,1,2,3,4,5,0,0,0,0,0,0,0,0,0,0},
{ 0,0,0,0,1,2,3,4,5,0,0,0,0,0,0,0,0,0},
{ 0,0,0,0,0,1,2,3,4,5,0,0,0,0,0,0,0,0},
{ 0,0,0,0,0,0,1,2,3,4,5,0,0,0,0,0,0,0},
{ 0,0,0,0,0,0,0,1,2,3,4,5,0,0,0,0,0,0},
{ 0,0,0,0,0,0,0,0,1,2,3,4,5,0,0,0,0,0},
{ 0,0,0,0,0,0,0,0,0,1,2,3,4,5,0,0,0,0},
{ 0,0,0,0,0,0,0,0,0,0,1,2,3,4,5,0,0,0},
{ 0,0,0,0,0,0,0,0,0,0,0,1,2,3,4,5,0,0},
{ 0,0,0,0,0,0,0,0,0,0,0,0,1,2,3,4,5,0},
{ 0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,3,4,5},
{ 5,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,3,4},
{ 4,5,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,3},
{ 3,4,5,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2},
{ 2,3,4,5,0,0,0,0,0,0,0,0,0,0,0,0,0,1}
 }; 

char* callsign[19]={
"OE3GOD", " 4U1UN", " VE8AT", "  W6WX", " KH6RS", "  ZL6B","VK6RBP","JA2IGY","  RR9O","  VR2B","  4S7B"," ZS6DN",
"  5Z4B", " 4X6TU", "  OH2B", "  CS3B", " LU4AA", "  OA4B", "  YV5B"}; 

char* qrg[6] = {"      ","14.100","18.110","21.150","24.930","28.200"};

void setup() {
  pinMode(BAND_PIN, INPUT);       // Pin 2 ist INT0
  digitalWrite(BAND_PIN, HIGH);   // interner Pull up Widerstand auf 5V
  attachInterrupt(1, interruptRoutine, LOW);

  DCF.Start();
  
  lcd.begin();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print(callsign[0]);
  lcd.setCursor(7,0);
  lcd.print("NCDXF Uhr");
  lcd.setCursor(0,1);
  lcd.print( F(__DATE__));
  delay(2000);

  setSyncProvider(RTC.get);
  // RTC.get();
    if (RTC.chipPresent()) {
      if (timeStatus()==timeNotSet) {
        lcd.setCursor(0,1);
        lcd.print("DS1307 not set !");
        delay(2500);
        }
      }
    else {
      lcd.setCursor(0,1);
      lcd.print("no DS1307 device");
      delay(2500);
      }
  
  setSyncInterval(900); //Update der Uhrzeit alle 15 Minuten 
  setSyncProvider(getDCFTime);
  
  lcd.setCursor(0,1);
  lcd.print("init DCF77      ");

  band = 1;

  while(timeStatus()==timeNotSet) {
    delay(1000);}
   lcd.clear();
   time_t t = now();
    if (t != 0) {
        RTC.set(t);   // set the RTC and the system time to the received value
    }}
    
void loop() {

  if(now() != prevDisplay) {
    verarbeiten();

    if (prevhour != hour()) {
      time_t t = now();
      if (t != 0) {
        RTC.set(t);   // set the RTC and the system time to the received value
        prevhour = hour();
        }
      }
    
    prevDisplay = now();
    digitalClockDisplay();
    }
  }

void digitalClockDisplay(){
//            1
//  0123456789012345
// "hh.mm.ss  28.200"
// "tt.mm fr  OE3GOD"

  lcd.setCursor(0,0);
  printDigits(hour());
  lcd.print(":");
  printDigits(minute());
  lcd.print(":");
  printDigits(second()); 
  
  lcd.setCursor(10,0);
  lcd.print(qrg[band]);

  lcd.setCursor(0,1);
  printDigits(day());
  lcd.print(".");
  printDigits(month());

  lcd.setCursor(6,1);  
  switch (weekday()) {
    case 1:
      lcd.print("SO");
    break;
    case 2:
      lcd.print("MO");
    break;
    case 3:
      lcd.print("DI");
    break;
    case 4:
      lcd.print("MI");
    break;
    case 5:
      lcd.print("DO");
    break;
    case 6:
      lcd.print("FR");
    break;
    case 7:
      lcd.print("SA");
    break;

    default:
      lcd.print("??");
    break;
    }
  lcd.setCursor(10,1);
  lcd.print(callsign[i]);
}

void printDigits(int digits){
  // utility function for digital clock display: prints leading 0
  if(digits < 10){
    lcd.print('0');
    }
  lcd.print(digits);
}

void SerialDigits(int digits){
  // utility function for digital clock display: prints leading 0
  if(digits < 10){
    Serial.print('0');
    }
  Serial.print(digits);
}

unsigned long getDCFTime() {
  time_t DCFtime = DCF.getUTCTime();
  if (DCFtime!=0) {
    lcd.clear();
    }
  return DCFtime;
}

void interruptRoutine() {
  if((millis() - alteZeit) > entprellZeit) { 
    // innerhalb der entprellZeit nichts machen
    alteZeit = millis(); // letzte Schaltzeit merken   
    band++;
    if (band > 5) {
      band = 1;
      }   
  verarbeiten();
  }
}

void verarbeiten() {
  minm = minute() % 3;
  sekm = int(second()/10);
  feld = (minm*60)+(sekm*10);
  spalte = (feld/10);

  for (i=0;i<19;i++) {
    if (matrix[i][spalte] == band) {
      break;
      }
    }
}


