
#include <Arduino.h>
#include "ArduinoJson.h"
#include "CommandProcessor.h"

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
  } else {
    Serial.println("Received unknown cmd: " + cmd);
  }
}