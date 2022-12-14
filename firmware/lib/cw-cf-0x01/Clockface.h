#pragma once

#include <Arduino.h>

#include "gfx/Super_Mario_Bros__24pt7b.h"

#include <ESP32-VirtualMatrixPanel-I2S-DMA.h>
#include <Tile.h>
#include <Locator.h>
#include <Game.h>
#include <Object.h>
#include <ImageUtils.h>
// Commons
#include <IClockface.h>
#include <CWDateTime.h>

#include "gfx/assets.h"
#include "gfx/mario.h"
#include "gfx/block.h"

class Clockface: public IClockface {
  private:
    CWDateTime* _dateTime;
    void updateTime();
    

  public:
    Clockface(VirtualMatrixPanel* display);
    void setup(CWDateTime *dateTime);
    void update(bool doNow = false);
    void externalEvent(int type);
    void init();
    enum DayNightType {
      Day = 0,
      Night,
    };
    void updateDayNight(DayNightType dayNightType = Day);
    DayNightType m_curDayNight = Day;
};
