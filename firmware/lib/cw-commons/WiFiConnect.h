#pragma once

#include "IOManager.h"


struct WiFiConnect 
{

 
    void connect() 
    {
      IOManager io;

      bool resp; 
      //wifiManager.resetSettings(); 


            
      
      if (!resp) {
        Serial.println("Failed to connect");
        delay(3000);
        ESP.restart();
      } else {
        Serial.println("connected!");
      }
    }
};
