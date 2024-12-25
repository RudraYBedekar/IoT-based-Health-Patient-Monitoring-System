
#include <LiquidCrystal.h>

LiquidCrystal lcd(13, 12, 11, 10, 9, 8);

#include <SoftwareSerial.h>
#define RX A0
#define TX A1
SoftwareSerial serial2(RX, TX);

#include <Wire.h>
#include "MAX30100_PulseOximeter.h"

#define REPORTING_PERIOD_MS     1000
PulseOximeter pox;

uint32_t tsLastReport = 0;
int current_heart, sop2;


String AP = "Redmi";
String PASS = "12345678";

String API = "CFU9SNJFJWX5M7TK";
String HOST = "184.106.153.149";
String PORT = "80";
int countTrueCommand;
int countTimeCommand;
boolean found = false;
int i = 0;

void onBeatDetected()
{
  Serial.println("Beat!");
    lcd.print(".");
}

void setup()
{
  Serial.begin(9600);

  serial2.begin(115200);
  Serial.println(" SPO2 Sensor ");

  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print(" HeartBeat ");
  lcd.setCursor(0, 1);
  lcd.print(" System");
  delay(3000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" Wifi Connecting");
  sendCommand("AT",5,"OK");
  sendCommand("AT+CWMODE=3",5,"OK");
  sendCommand("AT+CWJAP=\""+ AP +"\",\""+ PASS +"\"",20,"OK");
  lcd.clear();
  init_sensor();
}

void loop()
{

  lcd.setCursor(0, 0);
  lcd.print("Reading");
  i=0;current_heart=0;
 init_sensor();
  while(i<10)
  {
   read_spo2();
   if( current_heart>=50)
   {
        Serial.println(i);
    i++;
   }
    
    //delay(10);
  }
  lcd.setCursor(0, 1);
  lcd.print("Heart Beat:");
  lcd.print(current_heart);
  lcd.print("bpm     ");
  delay(3000);
  lcd.setCursor(0, 0);
  lcd.print("Updating On web  ");
  delay(3000);
  update_data();
  lcd.clear();
}


void init_sensor()
{

  Serial.print("Initializing pulse oximeter..");
  if (!pox.begin())
  {
    Serial.print("FAILED");
    for (;;);
  }
  else
  {
    Serial.println("SUCCESS");
  }
  pox.setOnBeatDetectedCallback(onBeatDetected);
}



void read_spo2()
{
  pox.update();
  if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
    Serial.print("Heart rate: ");
    Serial.print(pox.getHeartRate());
    current_heart = (pox.getHeartRate());
    Serial.print("  bpm / SpO2: ");
    Serial.print(pox.getSpO2());
    sop2 = (pox.getSpO2());
    Serial.println("%");

    tsLastReport = millis();
  }
  else sop2 = 0;
}

void update_data()
{

  String getData = "GET /update?api_key=" + API + "&" + "field1=" + String(current_heart);

  sendCommand("AT+CIPMUX=1", 5, "OK");
  sendCommand("AT+CIPSTART=4,\"TCP\",\"" + HOST + "\"," + PORT, 15, "OK");
  sendCommand("AT+CIPSEND=4," + String(getData.length() + 4), 4, ">");

  serial2.println(getData); delay(1500); countTrueCommand++;
  sendCommand("AT+CIPCLOSE=0", 5, "OK");
  delay(5000);
}






void sendCommand(String command, int maxTime, char readReplay[])
{
  Serial.print(countTrueCommand);
  Serial.print(". at command => ");
  Serial.print(command);
  Serial.print(" ");
  while (countTimeCommand < (maxTime * 1))
  {
    serial2.println(command);
    if (serial2.find(readReplay))
    {
      found = true;
      break;
    }

    countTimeCommand++;
  }

  if (found == true)
  {
    Serial.println("OK");
    countTrueCommand++;
    countTimeCommand = 0;
  }

  if (found == false)
  {
    Serial.println("Fail");
    countTrueCommand = 0;
    countTimeCommand = 0;
  }

  found = false;
}
