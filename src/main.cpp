#include <Arduino.h>
#include <Constants.h>
#include "wifi_setup.h"
#include "algorand/account.h"
#include "algorand/algoclient.h"
#include "CommandProcessor.h"

extern "C" {
    #include "crypto/base64.h"
}

String PUBLIC_KEY;
AlgoClient *CLIENT = new AlgoClient(Constants::BASE_URL, Constants::IDX_BASE_URL, Constants::API_KEY);
CommandProcessor *CMD_PROC = new CommandProcessor(CLIENT);
/*
* The setup funtion will be called once after boot.  Here we initialze
* WIFI and retrieve our public key. 
*/
void setup() {
  
  //Setting up serial output. This number should match monitor_speed in platformio.ini
  Serial.begin(115200);  

  //
  CMD_PROC->InitIO();

  setupWifi(Constants::MY_SSID.c_str(), Constants::MY_PASSWORD.c_str());
  CMD_PROC->UpdateLEDWifi(true);
  
  //Create an account object and retrieve the public key
  Account *account = new Account();
  PUBLIC_KEY = account->getPublicKey();

}

/*
*  Arduino will run this method in a loop.  All recurring tasks go here. 
*  In this method we will monitor our Algorand account 
*  for balance changes and device commands. 
*/
void loop() {

  //
  Serial.println("Using Address:");
  Serial.println(PUBLIC_KEY);
  //1) Lets check our balance for more ALGOS.  
  double balance = CLIENT->getAccountBalance(PUBLIC_KEY);
  Serial.print("Balance = ");
  Serial.print(balance);
  Serial.println(" ALGOS");

  /* 
    2) Check for any new commands
        In our scenario, a command is an instruction from a 
        user on the Algorand blockchain sent to our IoT 
        device via the note field.  
  */
  CMD_PROC->processCommands(PUBLIC_KEY);
  delay(15000);
}

