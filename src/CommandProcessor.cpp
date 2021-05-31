
#include <Arduino.h>
#include "ArduinoJson.h"
#include <Adafruit_NeoPixel.h>
#include "CommandProcessor.h"

const int OUTPUT_LED_WIFI_PIN = 13;
const int OUTPUT_SELECT_LOAD_PIN = 12;
const int OUTPUT_LOCK_PIN = 14;
const int OUTPUT_LOCK_LED_PIN = 25;
const int OUTPUT_UNLOCK_LED_PIN = 33;
const int INPUT_LOCK_SW_PIN = 27;
const int INPUT_UNLOCK_SW_PIN = 26;


/**
 * Initialize the Leds
 */ 
void CommandProcessor::InitIO(void){
  pinMode(OUTPUT_LED_WIFI_PIN, OUTPUT);
  pinMode(OUTPUT_SELECT_LOAD_PIN, OUTPUT);
  pinMode(OUTPUT_LOCK_PIN, OUTPUT);
  pinMode(OUTPUT_LOCK_LED_PIN, OUTPUT);
  pinMode(OUTPUT_UNLOCK_LED_PIN, OUTPUT); 
  pinMode(INPUT_LOCK_SW_PIN, INPUT);
  pinMode(INPUT_UNLOCK_SW_PIN, INPUT);

  digitalWrite(OUTPUT_LED_WIFI_PIN, LOW);
  digitalWrite(OUTPUT_SELECT_LOAD_PIN, LOW);
  digitalWrite(OUTPUT_LOCK_PIN, LOW);
  digitalWrite(OUTPUT_LOCK_LED_PIN, LOW);
  digitalWrite(OUTPUT_UNLOCK_LED_PIN, LOW);
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
 * Update Lock LED
 */ 
void CommandProcessor::UpdateLEDLock(void){
  bool state;
  state = digitalRead(INPUT_LOCK_SW_PIN);
  if(state) {
    digitalWrite(OUTPUT_LOCK_LED_PIN, LOW);
  } else {
    digitalWrite(OUTPUT_LOCK_LED_PIN, HIGH);
  }
}

/**
 * Update Unlock LED
 */ 
void CommandProcessor::UpdateLEDUnlock(void){
  bool state;
  state = digitalRead(INPUT_UNLOCK_SW_PIN);
  if(state) {
    digitalWrite(OUTPUT_UNLOCK_LED_PIN, LOW);
  } else {
    digitalWrite(OUTPUT_UNLOCK_LED_PIN, HIGH);
  }
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
  if(cmd.equalsIgnoreCase("reboot") || cmd.equalsIgnoreCase("restart")) {
    //ESP.restart();
  } //  else if(cmd.equalsIgnoreCase("lock")) {
  //   Serial.println("Locking Doors!");
  //     for(int i = 0; i < 3; i++) {
  //       UpdateOutputLock(true);
  //       delay(500);
  //       UpdateOutputLock(false);
  //       delay(500);
  //     }
  // } else if(cmd.equalsIgnoreCase("unlock")) {
  //   Serial.println("Unlocking Doors!");
  //     for(int i = 0; i < 3; i++) {
  //       UpdateOutputUnlock(true);
  //       delay(500);
  //       UpdateOutputUnlock(false);
  //       delay(500);
  //     }
  // } 
  else {
    Serial.println("Received unknown cmd: " + cmd);
  }
}