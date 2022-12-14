#pragma once

#include <Arduino.h>

class Sprite {

  protected:
  public:
    uint8_t _x;
    uint8_t _y;
    uint8_t _width;
    uint8_t _height;

    boolean collidedWith(Sprite* sprite);
    void logPosition();

    virtual const char* name();
};
