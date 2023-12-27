#include "DB.h"
#include <SD.h>
#include "Constants.h"

class DB{

    public:
    DB::DB(){
        getCurrentID();
    }

    bool get(uint32_t id,StaticJsonDocument& doc){
        
    }

    bool add(StaticJsonDocument& doc){

    }
    
    bool set(uint32_t id, String key, String value){

    }

    bool delete(uint32_t id){

    }

    private:
    uint32_t currentID;

    bool getCurrentID(){
        File stateFile;
        //open state
        stateFile = SD.open(STATEFILE, FILE_WRITE);
        if(!stateFile){
            Serial.println("Failed to open state file");
            return false;
        }
        long size = stateFile.size();
        StaticJsonDocument doc(size*2);
        auto error = deserializeJson(doc, stateFile);
        if (error) {
            Serial.print(F("deserializeJson() failed with code "));
            Serial.println(error.c_str());
            return false;
        }
        currentID = doc["currentID"].as<const uint32_t>();
        stateFile.close();  
        return true;
    }

};
