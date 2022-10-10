#include <Arduino.h>
#include "ESP8266WiFi.h"
#include <EEPROM.h>
#include <ESP8266HTTPClient.h>

const int VIRGIN_EEPROM = 255;

const int RESET_PIN = 5;

const int PIN_D2 = 4;
const int PIN_D3 = 0;
const int PIN_D5 = 14;
const int PIN_D6 = 12;
const int PIN_D7 = 13;

int restCallCounter = 0;

struct settings{
  char ssid[100];
  char password[100];
  char url[200];
} data;

void printSettings()
{
  if (data.ssid[0] != VIRGIN_EEPROM){
    Serial.println("Curent settings are:");
    Serial.printf("SSID='%s'\n", data.ssid);
    Serial.printf("Password='%s'\n", data.password);
    Serial.printf("Url='%s'\n", data.url);
  }
}


void getDataFromSerialConsole()
{

  Serial.setTimeout(600000);
  Serial.println("Enter SSID:");
  String tmp = Serial.readStringUntil('\n');
  tmp.trim();
  if (tmp.length() >0 ){
  strcpy(data.ssid, tmp.c_str());
   }
  Serial.println(data.ssid);
  Serial.println("Enter Password:");
  tmp = Serial.readStringUntil('\n');
  tmp.trim();
  if (tmp.length() > 0)
  {
    strcpy(data.password, tmp.c_str());
  }
  Serial.println(data.password);

  Serial.println("Enter Url:");
  tmp = Serial.readStringUntil('\n');
  tmp.trim();
  if (tmp.length() >0 ){
    strcpy(data.url, tmp.c_str());
  }
  Serial.println(data.url);

  printSettings();
  Serial.println("Is this correct? [y|n]");

  tmp = Serial.readStringUntil('\n');
  tmp.trim();
  if (tmp[0] == 'y')
  {
    EEPROM.put(0, data);
    EEPROM.commit();
    Serial.println("New data saved");
  }
  else
  {
    getDataFromSerialConsole();
  }
}

void connectToWifi() {
  WiFi.persistent(false);
  WiFi.begin(data.ssid, data.password);
  int i = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    Serial.print(++i);
    Serial.print(' ');
    }

  Serial.println('\n');
  Serial.println("Connection established!");  
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());
  digitalWrite(LED_BUILTIN, LOW);
}

void setup()
{
  restCallCounter = 0;
  Serial.begin(115200);
  delay(10);
  Serial.println('\n');
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(RESET_PIN, OUTPUT);
  digitalWrite(RESET_PIN, HIGH);
  pinMode(PIN_D2, OUTPUT);
  digitalWrite(PIN_D2, LOW);
  pinMode(PIN_D3, OUTPUT);
  digitalWrite(PIN_D3,LOW);
  pinMode(PIN_D5, OUTPUT);
  digitalWrite(PIN_D5,LOW);
  pinMode(PIN_D6, OUTPUT);
  digitalWrite(PIN_D6,LOW);
  pinMode(PIN_D7, OUTPUT);
  digitalWrite(PIN_D7,LOW);

  EEPROM.begin(400);
  EEPROM.get(0, data);

  if (digitalRead(RESET_PIN) == LOW || data.ssid[0] == VIRGIN_EEPROM || strlen(data.ssid) == 0 || strlen(data.password) == 0 || strlen(data.url) == 0)
  {
    getDataFromSerialConsole();

  } else {
    printSettings();
  }
}

void callUrl()
{
  HTTPClient http;
  WiFiClient client;
  http.setReuse(true);
  http.setTimeout(500);
  http.begin(client, data.url);
  int httpCode = http.GET();
  Serial.printf("HTTP-Code %i\n", httpCode);
  if (httpCode == HTTP_CODE_OK)
  {
    String payload = http.getString();
    Serial.println(payload);
    client.stop();
    http.end();
  }
  else
  {
    int i = 0;
    while (i < 10)
    {
      digitalWrite(LED_BUILTIN, LOW);
      delay(100);
      digitalWrite(LED_BUILTIN, HIGH);
      delay(100);
      i++;
    }
    http.end();
    if (restCallCounter < 5)
      {
      restCallCounter++;
      callUrl();
      }
  }
}

void loop()
{
  Serial.println("State of Pin D2: ");
  Serial.println(digitalRead(PIN_D2));
  Serial.println("State of Pin D3: ");
  Serial.println(digitalRead(PIN_D3));  
  Serial.println("State of Pin D5: ");
  Serial.println(digitalRead(PIN_D5));
  Serial.println("State of Pin D6: ");
  Serial.println(digitalRead(PIN_D6)); 
  Serial.println("State of Pin D7: ");
  Serial.println(digitalRead(PIN_D7));

  Serial.println("Going to deep sleep");

  bool sensorIsOpen = digitalRead(PIN_D2) == HIGH || digitalRead(PIN_D3) == HIGH || digitalRead(PIN_D5) == HIGH || digitalRead(PIN_D6) == HIGH || digitalRead(PIN_D7) == HIGH;
  //Check if any sesnor is open
  if (sensorIsOpen){
    connectToWifi();
    restCallCounter = 0;
    callUrl();
    ESP.deepSleep(5e6); // 1 min
  } else {
    ESP.deepSleep(5e6); //5 min
  }

  
  // delay(3000);


}