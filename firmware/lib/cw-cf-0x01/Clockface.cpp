
#include "Clockface.h"

EventBus eventBus;

const char* FORMAT_TWO_DIGITS = "%02d";

// Graphical elements
Tile ground(GROUND, 8, 8);  // ground 8*8

Object bush(BUSH, 21, 9); // bush 21*9
Object cloud1(CLOUD1, 13, 12); // cloud 13*12
Object cloud2(CLOUD2, 13, 12); // cloud 13*12
Object cloud3(CLOUD1, 13, 12); // cloud 13*12
Object cloud4(CLOUD2, 13, 12); // cloud 13*12
Object hill(HILL, 20, 22); // hill 20*22

// 'mario-idle', 13x16px  
// 'mario-jump', 17x16px
Mario mario; // 41~54

// Block 19*19
Block hourBlock; 
Block minuteBlock;

unsigned long lastMillis = 0;

Clockface::Clockface(VirtualMatrixPanel* display) {

  Locator::provide(display);
  Locator::provide(&eventBus);
}

void Clockface::setup(CWDateTime *dateTime) {
  _dateTime = dateTime;

  Locator::getDisplay()->setFont(&Super_Mario_Bros__24pt7b);
  Locator::getDisplay()->fillRect(0, 0, Locator::getDisplay()->width(), Locator::getDisplay()->height(), SKY_COLOR);

  ground.fillRow(Locator::getDisplay()->height() - ground._height);

  hill.draw(0, Locator::getDisplay()->height()-ground._height-hill._height);
  bush.draw(Locator::getDisplay()->width()-21, Locator::getDisplay()->height()-ground._height-bush._height);
  cloud1.draw(0, 21);
  cloud2.draw(51, 7);
  cloud3.draw(51+13, 7);
  cloud4.draw(Locator::getDisplay()->width()-13, 33);

  updateTime();

  hourBlock.init(Locator::getDisplay()->width()/2-19, Locator::getDisplay()->height()-56);
  minuteBlock.init(Locator::getDisplay()->width()/2, Locator::getDisplay()->height()-56);
  mario.init((Locator::getDisplay()->width()-MARIO_IDLE_SIZE[0])/2, Locator::getDisplay()->height()-ground._height-mario._height);
}

void Clockface::init()
{

}

void Clockface::updateDayNight(DayNightType dayNightType )
{
  if (dayNightType == m_curDayNight)
    return;
  
  m_curDayNight = dayNightType;


  // SKY_COLOR = m_curDayNight == Day ? SKY_COLOR_Day : SKY_COLOR_Night;
  // _MASK = SKY_COLOR;

  Locator::getDisplay()->fillRect(0, 0, Locator::getDisplay()->width(), Locator::getDisplay()->height(), SKY_COLOR);

  hill.draw(0, Locator::getDisplay()->height()-ground._height-hill._height);
  bush.draw(Locator::getDisplay()->width()-21, Locator::getDisplay()->height()-ground._height-bush._height);
  cloud1.draw(0, 21);
  cloud2.draw(51, 7);
  cloud3.draw(51+13, 7);
  cloud4.draw(Locator::getDisplay()->width()-13, 33);

  updateTime();

  update();

}


int preDate = -1;

void Clockface::update(bool doNow) {
  if (hourBlock.state() != Block::IDLE) {
    hourBlock.update();
  }
  if (minuteBlock.state() != Block::IDLE) {
    minuteBlock.update();
  }
  if (mario.state() != Mario::IDLE) {
    mario.update();
  }

  if (doNow || _dateTime->getSecond() == 0 && millis() - lastMillis > 1000 ) {
    mario.jump();
    updateTime();
    lastMillis = millis();

    if (m_curDayNight == Day && (_dateTime->getHour() < 6 || _dateTime->getHour() > 18)) {
      updateDayNight(Night);
    } else if (m_curDayNight == Night && (_dateTime->getHour() >=6 && _dateTime->getHour() <= 18)) {
      updateDayNight(Day);
    }

    Serial.println(_dateTime->getFormattedTime());

    if (preDate != _dateTime->getSecond()) {
      preDate = _dateTime->getSecond();

      // update DateTime
        // virtualDisp->printUTF8("2022年06月");
  // virtualDisp->setTextSize(3);
  // virtualDisp->setCursor(15,40);
  // virtualDisp->printUTF8("10");
  // virtualDisp->setTextSize(1); 
  // virtualDisp->setCursor(65,68);
  // virtualDisp->printUTF8("日");
      Locator::getDisplay()->print("");
    }

  }
}

void Clockface::updateTime() {
  hourBlock.setText(String(_dateTime->getHour()));
  minuteBlock.setText(String(_dateTime->getMinute(FORMAT_TWO_DIGITS)));
}

void Clockface::externalEvent(int type) {
  if (type == 0) {  //TODO create an enum
    mario.jump();
    updateTime();
  }
}
