#pragma once

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include "EventBus.h"
#include <ESP32-VirtualMatrixPanel-I2S-DMA.h>

class Locator {
  private: 
    static VirtualMatrixPanel* _display;
    static EventBus* _eventBus;

  public:    
    static VirtualMatrixPanel* getDisplay();
    static EventBus* getEventBus();
    static void provide(VirtualMatrixPanel* display);
    static void provide(EventBus* eventBus);
};
