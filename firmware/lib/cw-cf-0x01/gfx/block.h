#pragma once

#include <Arduino.h>
#include <Game.h>
#include <Locator.h>
#include <EventTask.h>
#include "assets.h"

const uint8_t MOVE_PACE = 2;
const uint8_t MAX_MOVE_HEIGHT = 4;

class Block: public Sprite, public EventTask {
  public:
    enum State {
      IDLE,
      HIT
    };   
  private:

    Direction direction; 

    String _text;

    unsigned long lastMillis = 0;
    State _state = IDLE; 
    State _lastState = IDLE; 
    uint8_t _lastY;
    uint8_t _firstY;
    
    void idle();
    void hit();
    void setTextBlock();

  public:
    Block();
    void setText(String text);
    void init(int x, int y);
    void update();    
    const char* name();
    void execute(EventType event, Sprite* caller);
    State state() { return _state; }
};
