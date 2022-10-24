#include <Arduino.h>
#include "ESP8266WiFi.h"
#include <EEPROM.h>
#include <ESP8266HTTPClient.h>
#include <string>

const int VIRGIN_EEPROM = 255;
#define RTCMEMORYSTART 65
#define SLEEP_SHORT 30 * 1000000
#define SLEEP_LONG 300 * 1000000

const int RESET_PIN = 5;

const int PIN_D2 = 4;
const int PIN_d1 = 0;
const int PIN_D5 = 14;
const int PIN_D6 = 12;
const int PIN_D7 = 13;

int restCallCounter = 0;

struct settings
{
  char ssid[100];
  char password[100];
  char url[200];
  bool d2Activated;
  bool d1Activated;
  bool d5Activated;
  bool d6Activated;
  bool d7Activated;
} data;

typedef struct
{
  bool init = 0;
  bool d1;
  bool d2;
  bool d5;
  bool d6;
  bool d7;

  bool d1Activated;
  bool d2Activated;
  bool d5Activated;
  bool d6Activated;
  bool d7Activated;
} rtcStore;

rtcStore rtcValues;

void printSettings()
{
  if (data.ssid[0] != VIRGIN_EEPROM)
  {
    Serial.println("Current settings are:");
    Serial.printf("SSID='%s'\n", data.ssid);
    Serial.printf("Password='%s'\n", data.password);
    Serial.printf("Url='%s'\n", data.url);
    Serial.printf("D1: %i, D2: %i, D5: %i , D6: %i, D7: %i\n", data.d1Activated, data.d2Activated, data.d5Activated, data.d6Activated, data.d7Activated);
  }
}

void getDataFromSerialConsole()
{
  Serial.setTimeout(600000);
  Serial.println("Enter SSID:");
  String tmp = Serial.readStringUntil('\n');
  tmp.trim();
  if (tmp.length() > 0)
  {
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
  if (tmp.length() > 0)
  {
    strcpy(data.url, tmp.c_str());
  }
  Serial.println(data.url);

  Serial.println("Should Pin d1 be observed? [y|n] ");
  tmp = Serial.readStringUntil('\n');
  data.d1Activated = ('y' == tmp[0]) ? true : false;
  Serial.println("Should Pin D2 be observed? [y|n] ");
  tmp = Serial.readStringUntil('\n');
  data.d2Activated = ('y' == tmp[0]) ? true : false;
  Serial.println("Should Pin D5 be observed? [y|n] ");
  tmp = Serial.readStringUntil('\n');
  data.d5Activated = ('y' == tmp[0]) ? true : false;
  Serial.println("Should Pin D6 be observed? [y|n] ");
  tmp = Serial.readStringUntil('\n');
  data.d6Activated = ('y' == tmp[0]) ? true : false;

  Serial.println("Should Pin D7 be observed? [y|n] ");
  tmp = Serial.readStringUntil('\n');
  data.d7Activated = ('y' == tmp[0]) ? true : false;

  Serial.printf("D1: %i, D2: %i, D5: %i, D6: %i, D7: %i\n", data.d1Activated, data.d2Activated, data.d5Activated, data.d6Activated, data.d7Activated);

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

void connectToWifi()
{
  WiFi.persistent(false);
  Serial.printf("Connecting to '%s'\n", data.ssid);
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

void readFromRTCMemory()
{
  system_rtc_mem_read(RTCMEMORYSTART, &rtcValues, sizeof(rtcValues));
  yield();
}

void writeToRTCMemory()
{
  system_rtc_mem_write(RTCMEMORYSTART, &rtcValues, sizeof(rtcValues));
  yield();
}

void initRtcDataOrGetFromUser()
{
  Serial.println("Data not initialized or not set!");
  EEPROM.begin(405);
  EEPROM.get(0, data);
  if (digitalRead(RESET_PIN) == LOW || data.ssid[0] == VIRGIN_EEPROM || strlen(data.ssid) == 0 || strlen(data.password) == 0 || strlen(data.url) == 0)
  {
    getDataFromSerialConsole();
  }
  rtcValues.d1Activated = data.d1Activated;
  rtcValues.d2Activated = data.d2Activated;
  rtcValues.d5Activated = data.d5Activated;
  rtcValues.d6Activated = data.d6Activated;
  rtcValues.d7Activated = data.d7Activated;
  rtcValues.init = true;
  writeToRTCMemory();
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
  digitalWrite(PIN_D2, HIGH);
  pinMode(PIN_d1, OUTPUT);
  digitalWrite(PIN_d1, HIGH);
  pinMode(PIN_D5, OUTPUT);
  digitalWrite(PIN_D5, HIGH);
  pinMode(PIN_D6, OUTPUT);
  digitalWrite(PIN_D6, HIGH);
  pinMode(PIN_D7, OUTPUT);
  digitalWrite(PIN_D7, HIGH);

  readFromRTCMemory();

  if (!rtcValues.init || digitalRead(RESET_PIN) == LOW)
  {
    initRtcDataOrGetFromUser();
  }
  else
  {
    Serial.println("Data already initialized.");
  }
}

void callUrl()
{
  HTTPClient http;
  WiFiClient client;
  http.setReuse(true);
  http.setTimeout(500);

  http.begin(client, data.url);
  http.addHeader("Content-Type", "text/plain");

  int httpCode = http.POST("ON");
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
    client.stop();
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

  readFromRTCMemory();

  Serial.printf("Old states: D1: %i, D2: %i, D5: %i, D6: %i, D7: %i\n", rtcValues.d1, rtcValues.d2, rtcValues.d5, rtcValues.d6, rtcValues.d7);
  int d2 = digitalRead(PIN_D2);
  int d1 = digitalRead(PIN_d1);
  int d5 = digitalRead(PIN_D5);
  int d6 = digitalRead(PIN_D6);
  int d7 = digitalRead(PIN_D7);
  Serial.printf("New states: D1: %i, D2: %i, D5: %i, D6: %i, D7: %i\n", d1, d2, d5, d6, d7);
  Serial.printf("Checking for states D1: %i, D2: %i, D5: %i , D6: %i, D7: %i\n", rtcValues.d1Activated, rtcValues.d2Activated, rtcValues.d5Activated, rtcValues.d6Activated, rtcValues.d7Activated);

  bool anySensorIsOpenAndWasBefore = (d1 & rtcValues.d1 & rtcValues.d1Activated) ||
                                     (d2 & rtcValues.d2 & rtcValues.d2Activated) ||
                                     (d5 & rtcValues.d5 & rtcValues.d5Activated) ||
                                     (d6 & rtcValues.d6 & rtcValues.d6Activated) ||
                                     (d7 & rtcValues.d7 & rtcValues.d7Activated);

  bool anySensorIsOpenAndWasClosedBefore = (d1 & rtcValues.d1Activated) ||
                                           (d2 & rtcValues.d2Activated) ||
                                           (d5 & rtcValues.d5Activated) ||
                                           (d6 & rtcValues.d6Activated) ||
                                           (d7 & rtcValues.d7Activated);
  // TODO Check if was open, but is now closed (0 -> 1, 1 -> 0)
  Serial.printf("anySensorIsOpenAndWasBefore = %i\n", anySensorIsOpenAndWasBefore);
  Serial.printf("anySensorIsOpenAndWasClosedBefore = %i\n", anySensorIsOpenAndWasClosedBefore);

  rtcValues.d1 = d1;
  rtcValues.d2 = d2;
  rtcValues.d5 = d5;
  rtcValues.d6 = d6;
  rtcValues.d7 = d7;
  writeToRTCMemory();

  Serial.print("Going to deep sleep for ");
  // Check if any sensor is open
  if (anySensorIsOpenAndWasBefore)
  {
    Serial.println("30 s");
    EEPROM.begin(405);
    EEPROM.get(0, data);
    connectToWifi();
    restCallCounter = 0;
    // callUrl();
    // ESP.deepSleep(SLEEP_SHORT); // 30s
  }
  else if (anySensorIsOpenAndWasClosedBefore)
  {
    Serial.println("30 s");
    // ESP.deepSleep(SLEEP_SHORT); // 30s
  }
  else
  {
    Serial.println("5 mins");
    // ESP.deepSleep(SLEEP_LONG); // 5 min /
  }

  delay(10000); // FIXME
}