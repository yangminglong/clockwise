#pragma once

#include <Arduino.h>
#include <Game.h>
#include <Locator.h>
#include <EventBus.h>
#include <ImageUtils.h>
#include "assets.h"


const uint8_t MARIO_PACE = 3;
const uint8_t MARIO_JUMP_HEIGHT = 14;

// 'mario-idle', 13x16px  
// 'mario-jump', 17x16px

class Mario: public Sprite, public EventTask {
  public:
    enum State {
      IDLE,
      WALKING,
      JUMPING
    };
  private:


    Direction direction;

    int _lastX;
    int _lastY;

    const unsigned short* _sprite;
    unsigned long lastMillis = 0;
    State _state = IDLE; 
    State _lastState = IDLE; 
    
    void idle();

  public:
    Mario();
    void init(int x, int y);
    void move(Direction dir, int times);
    void jump();
    void update();
    const char* name();
    void execute(EventType event, Sprite* caller);
    State state() {return _state; } 

};
