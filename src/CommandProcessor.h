
#ifndef COMMAND_PROCESSOR_H
#define COMMAND_PROCESSOR_H

#include "algorand/algoclient.h"

extern "C" {
    #include "crypto/base64.h"
    //int base64_decode(const unsigned char*, size_t, size_t*);
}

class CommandProcessor {

    public: 
        CommandProcessor(AlgoClient *client);
        void processCommands(String pubKey);
        void InitIO(void);
        //void UpdateOutputUnlock(bool state);
        //void UpdateOutputLock(bool state);
        void UpdateLEDWifi(bool state);
        void UpdateLEDLock(void);
        void UpdateLEDUnlock(void);
    private:
        String txID;
        AlgoClient *client;
        void processCmd(String cmd);
        
};

#endif