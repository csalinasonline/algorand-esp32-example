
#include <HardwareSerial.h>
#include "algorand/algoclient.h"
#include "ArduinoJson.h"
#include <HTTPClient.h>

extern "C" {
    #include "crypto/base64.h"
}

/**
 * The AlgoClient is primarily responsible for managing APIs to interact 
 * with the blockchain 
 */ 
AlgoClient::AlgoClient(String v2Url, String v2IdxUrl, String aKey){
    baseV2Url = v2Url;
    baseV2IndexerUrl = v2IdxUrl;
    apiKey = aKey;
}

/**
 * Retrieve genesis hash
 * void
 * return string genesis hash
 */ 
String AlgoClient::getGenesisHash(void) {
    String serverPath = baseV2Url;
    int len = serverPath.length();     
    String serverPath2 = serverPath.substring(0,len-3) + "versions/";  // remove v2
    DynamicJsonDocument doc = get(serverPath2);
    String genesis_hash = doc["genesis_hash_b64"]; 
    return genesis_hash;
}

/**
 * Retrieve genesis ID
 * void
 * return string genesis ID
 */ 
String AlgoClient::getGenesisID(void) {
    String serverPath = baseV2Url;
    int len = serverPath.length();     
    String serverPath2 = serverPath.substring(0,len-3) + "versions/";  // remove v2
    DynamicJsonDocument doc = get(serverPath2);
    String genesis_id = doc["genesis_id"]; 
    return genesis_id;
}

/**
 * Retrieve version
 * void
 * return string version
 */ 
String AlgoClient::getVersion(void) {
    String serverPath = baseV2Url;
    int len = serverPath.length();     
    String serverPath2 = serverPath.substring(0,len-3) + "versions/";  // remove v2
    DynamicJsonDocument doc = get(serverPath2);
    int doc_len = doc["versions"].size() ; 
    String version = "";
    if(doc_len > 0) {  // get supported versions
        if(doc_len == 1) {
            String temp = doc["versions"][0];
            version = temp;
        }
        else {
            String temp = doc["versions"][0];
            String temp2 = doc["versions"][1];
            version = temp + "," + temp2;
        }
    }
    return version;
}

/**
 * Retrieve account information and extract the balance
 * String publicKey
 * return double balance
 */ 
double AlgoClient::getAccountBalance(String pubKey) {
    String serverPath = baseV2Url + "accounts/"+pubKey;
    DynamicJsonDocument doc = get(serverPath);
    double balance = doc["amount"];
    if(balance > 0) {
        balance = balance/1000000;
    }
    return balance;
}

/**
 * Retrieve transactions from the Indexer
 * String publicKey
 * int limit -- the number of results to return
 */ 
DynamicJsonDocument AlgoClient::getTransactions(String pubKey, int limit){
    String serverPath = baseV2IndexerUrl + "accounts/"+ pubKey 
                        + "/transactions?limit=" + limit;
    return get(serverPath);
}

/**
 * Common method for API GET requests 
 */
DynamicJsonDocument AlgoClient::get(String serverPath){
    DynamicJsonDocument *doc = NULL;
    if(WiFi.status()== WL_CONNECTED) {
        try {
            HTTPClient http;

            http.begin(serverPath.c_str());
            http.addHeader("x-api-key", apiKey);
        
            // Send HTTP GET request
            int httpResponseCode = http.GET();
        
            if (httpResponseCode>0) {
                Serial.print("HTTP Response code: ");
                Serial.println(httpResponseCode);
                String payload = http.getString();
                
                doc = new DynamicJsonDocument(payload.length());
                deserializeJson(*doc, payload);
                //Serial.println(payload);
            }
            else {
                Serial.print("Error code: ");
                Serial.println(httpResponseCode);
            }
            // Free resources
            http.end();            
        } 
        catch(const std::exception& e){
            Serial.print(e.what());
        }
        return *doc;
    }
    else {
      Serial.println("WiFi Disconnected");
      return *doc;
    }
}

 