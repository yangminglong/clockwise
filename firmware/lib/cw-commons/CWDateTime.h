#pragma once
#include <Arduino.h>
#include <WiFi.h>


class CWDateTime
{
private:

public:
  void begin();
  void setTimezone(const char *timeZone);
  String getTimezone();
  void update();
  String getFormattedTime();

  char *getHour(const char *format);
  char *getMinute(const char *format);
  int getHour();
  int getMinute();
  int getSecond();
  long getMilliseconds();

  int getDay();
  int getMonth();
  int getWeekday();


};
