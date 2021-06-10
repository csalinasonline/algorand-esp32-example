
#include <Arduino.h>
#include "ArduinoJson.h"
#include <Adafruit_NeoPixel.h>
#include "CommandProcessor.h"

//
const int OUTPUT_LED_WIFI_PIN = 23;
const int OUTPUT_LAUNCH_PIN = 22;
const int OUTPUT_BUZZ_PIN = 12;

//
#define DEF_LAUNCH_DISABLED     HIGH
#define DEF_LAUNCH_ENABLED      LOW
#define DEF_MULTISHOT_MAX_RUNS  10
#define DEF_MULTISHOT_DELAY     7000

/**
 * Initialize the Leds
 */ 
void CommandProcessor::InitIO(void){
  pinMode(OUTPUT_LED_WIFI_PIN, OUTPUT);
  pinMode(OUTPUT_LAUNCH_PIN, OUTPUT);
  pinMode(OUTPUT_BUZZ_PIN, OUTPUT);   

  digitalWrite(OUTPUT_LED_WIFI_PIN, LOW);
  digitalWrite(OUTPUT_LAUNCH_PIN, DEF_LAUNCH_DISABLED);
  digitalWrite(OUTPUT_BUZZ_PIN, LOW);  
}

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
 * Update Launch
 */ 
void CommandProcessor::UpdateLaunch(bool state){
  if(state == true)
    digitalWrite(OUTPUT_LAUNCH_PIN,DEF_LAUNCH_ENABLED);
  else
    digitalWrite(OUTPUT_LAUNCH_PIN, DEF_LAUNCH_DISABLED);
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
  if(cmd.equalsIgnoreCase("reboot") || cmd.equalsIgnoreCase("restart")) {
    //ESP.restart();
  } 
  else if(cmd.equalsIgnoreCase("launch_single")) {
    UpdateLaunch(DEF_LAUNCH_ENABLED);
    UpdateBuzz(true);
    delay(1000);
    UpdateLaunch(DEF_LAUNCH_DISABLED);
    UpdateBuzz(false);
  }
  else if(cmd.equalsIgnoreCase("launch_multi")) {
    for(int i = 0; i < DEF_MULTISHOT_MAX_RUNS; i++) {
      UpdateLaunch(DEF_LAUNCH_ENABLED);
      UpdateBuzz(true);
      delay(1000);
      UpdateBuzz(false);
      UpdateLaunch(DEF_LAUNCH_DISABLED);
      //
      delay(DEF_MULTISHOT_DELAY);
    }
  }
  else {
    Serial.println("Received unknown cmd: " + cmd);
  }
}