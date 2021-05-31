
#include <Arduino.h>
#include "ArduinoJson.h"
#include <Adafruit_NeoPixel.h>
#include "CommandProcessor.h"

//
const int OUTPUT_LED_WIFI_PIN = 13;
const int OUTPUT_SELECT_LOAD_PIN = 12;
const int OUTPUT_LOCK_PIN = 14;
const int OUTPUT_LOCK_LED_PIN = 25;
const int OUTPUT_UNLOCK_LED_PIN = 33;
const int OUTPUT_BUZZ_PIN = 32;
const int INPUT_LOCK_SW_PIN = 27;
const int INPUT_UNLOCK_SW_PIN = 26;

//
#define DEF_LOCK              LOW
#define DEF_UNLOCK            HIGH
#define DEF_SELECT_DISABLED   HIGH
#define DEF_SELECT_ENABLED    LOW
#define DEF_LOCK_DISABLED     HIGH
#define DEF_LOCK_ENABLED      LOW


/**
 * Initialize the Leds
 */ 
void CommandProcessor::InitIO(void){
  pinMode(OUTPUT_LED_WIFI_PIN, OUTPUT);
  pinMode(OUTPUT_SELECT_LOAD_PIN, OUTPUT);
  pinMode(OUTPUT_LOCK_PIN, OUTPUT);
  pinMode(OUTPUT_LOCK_LED_PIN, OUTPUT); 
  pinMode(OUTPUT_UNLOCK_LED_PIN, OUTPUT); 
  pinMode(OUTPUT_BUZZ_PIN, OUTPUT);   
  pinMode(INPUT_LOCK_SW_PIN, INPUT_PULLUP);
  pinMode(INPUT_UNLOCK_SW_PIN, INPUT_PULLUP);

  digitalWrite(OUTPUT_LED_WIFI_PIN, LOW);
  digitalWrite(OUTPUT_SELECT_LOAD_PIN, DEF_SELECT_DISABLED);
  digitalWrite(OUTPUT_LOCK_PIN, DEF_LOCK_DISABLED);
  digitalWrite(OUTPUT_LOCK_LED_PIN, LOW);
  digitalWrite(OUTPUT_UNLOCK_LED_PIN, LOW);
  digitalWrite(OUTPUT_BUZZ_PIN, LOW);  
}

/**
 * Update Output Unlock
 */ 
// void CommandProcessor::UpdateOutputUnlock(bool state){
//   if(state != true)
//     digitalWrite(OUTPUT_UNLOCK_PIN,HIGH);
//   else
//     digitalWrite(OUTPUT_UNLOCK_PIN, LOW);
// }

/**
 * Update Output Lock
 */ 
// void CommandProcessor::UpdateOutputLock(bool state){
//   if(state != true)
//     digitalWrite(OUTPUT_LOCK_PIN,HIGH);
//   else
//     digitalWrite(OUTPUT_LOCK_PIN, LOW);
// }

/**
 * Update Wifi LED
 */ 
void CommandProcessor::UpdateLEDWifi(bool state){
  if(state == true)
    digitalWrite(OUTPUT_LED_WIFI_PIN,HIGH);
  else
    digitalWrite(OUTPUT_LED_WIFI_PIN, LOW);
}

/**
 * Update Unlock/Lock LED
 */ 
void CommandProcessor::UpdateLEDUnlockLock(void){
  bool state;
  state = digitalRead(INPUT_UNLOCK_SW_PIN);
  if(state) {
    digitalWrite(OUTPUT_UNLOCK_LED_PIN, HIGH);
    digitalWrite(OUTPUT_LOCK_LED_PIN, LOW);
  } else {
    digitalWrite(OUTPUT_UNLOCK_LED_PIN, LOW);
    digitalWrite(OUTPUT_LOCK_LED_PIN, HIGH);
  }
}

/**
 * Update Select Load
 */ 
void CommandProcessor::UpdateSelectLoad(bool state){
  if(state == true)
    digitalWrite(OUTPUT_SELECT_LOAD_PIN,DEF_SELECT_ENABLED);
  else
    digitalWrite(OUTPUT_SELECT_LOAD_PIN, DEF_SELECT_DISABLED);
}

/**
 * Update Lock
 */ 
void CommandProcessor::UpdateLock(bool state){
  if(state == true)
    digitalWrite(OUTPUT_LOCK_PIN,DEF_LOCK_ENABLED);
  else
    digitalWrite(OUTPUT_LOCK_PIN,DEF_LOCK_DISABLED);
}

/**
 * Update Buzzer
 */ 
void CommandProcessor::UpdateBuzz(bool state){
  if(state == true)
    digitalWrite(OUTPUT_BUZZ_PIN,HIGH);
  else
    digitalWrite(OUTPUT_BUZZ_PIN, LOW);
}

/**
 * Initialize the AlgoClient  
 */ 
CommandProcessor::CommandProcessor(AlgoClient *algoClient){
    client = algoClient;
}

/**
 *  The processCommands function will retrieve the last transaction for the provided publicKey.
 *  It then parses the note field looking for a JSON object with an instruction for the device.
 *  We assume the last transaction contains the last known state of the device so whatever command
 *  exists we will reset device state accordingly. 
 */
void CommandProcessor::processCommands(String pubKey) {
  static bool one_shot = false;
    //
    // So we only care about the last transation (i.e. limit=1)  
    DynamicJsonDocument doc = client->getTransactions(pubKey, 1);
    try {
          if(doc != NULL && doc["transactions"][0] != NULL 
              && !this->txID.equalsIgnoreCase(doc["transactions"][0]["id"].as<String>()) ) {
              // Set this transaction ID so we don't process it again
              this->txID = doc["transactions"][0]["id"].as<String>();
              size_t outlen ;
              String note64 = doc["transactions"][0]["note"];
              if(note64 != NULL) {
                unsigned char* decodedNote = base64_decode(((const unsigned char *)note64.c_str()), note64.length(), &outlen);
                //Important! No defensive programming. We assume the note is received in JSON format.
                DynamicJsonDocument noteDoc(64);
                deserializeJson(noteDoc, decodedNote);
                if(!one_shot) {
                  one_shot = true;
                }
                else {
                  if(noteDoc.containsKey("cmd"))  {
                    processCmd(noteDoc["cmd"].as<String>());
                  }
                }
              }
          }
        }
        catch(const std::exception& e){
            Serial.print(e.what());
        } catch(...){
            Serial.print("catch all");
        } 
}

/*
* Not tested. Don't use without additional defensive programming around the transaction ID
* otherwise this will result in constant reboots. 
*/
void CommandProcessor::processCmd(String cmd){
  bool state;
  if(cmd.equalsIgnoreCase("reboot") || cmd.equalsIgnoreCase("restart")) {
    //ESP.restart();
  } 
  else if(cmd.equalsIgnoreCase("lock")) {
    state = digitalRead(INPUT_UNLOCK_SW_PIN);
    if(state == true) {
      Serial.println("Locking Doors!");
      UpdateLock(DEF_LOCK);
      delay(500);
      UpdateSelectLoad(true);
      UpdateBuzz(true);
      delay(500);
      UpdateSelectLoad(false);
      UpdateBuzz(false);
    }
    else {
      Serial.println("Doors is Already Locked!");
      for(int i = 0; i < 2; i++) {
        UpdateBuzz(true);
        delay(300);
        UpdateBuzz(false);
        delay(300);
      }
    }
  } else if(cmd.equalsIgnoreCase("unlock")) {
    state = digitalRead(INPUT_UNLOCK_SW_PIN);
    if(state == false) {
      Serial.println("Unlocking Doors!");
      UpdateLock(DEF_UNLOCK);
      delay(500);
      UpdateSelectLoad(true);
      UpdateBuzz(true);
      delay(500);
      UpdateSelectLoad(false);
      UpdateBuzz(false);
    }
    else {
      Serial.println("Doors is Already Unlocked!");
      for(int i = 0; i < 3; i++) {
        UpdateBuzz(true);
        delay(300);
        UpdateBuzz(false);
        delay(300);
      }
    }
  }
  else {
    Serial.println("Received unknown cmd: " + cmd);
  }
}