#include "CWDateTime.h"
#include <ESPNtpClient.h>

void CWDateTime::begin()
{
}





void CWDateTime::update()
{
}

String CWDateTime::getFormattedTime()
{
  String dateTimeStr = NTP.getTimeDateString(time(NULL), "%02y年%02m月%02d日%02H:%02M:%02S");

  return dateTimeStr;
}

char *CWDateTime::getHour(const char *format)
{
  static char buffer[3] = {'\0'};
  strncpy(buffer, NTP.getTimeDateString(time(NULL),"%02H"), sizeof(buffer));
  return buffer;
}

char *CWDateTime::getMinute(const char *format)
{
  static char buffer[3] = {'\0'};
  strncpy(buffer, NTP.getTimeDateString(time(NULL),"%02M"), sizeof(buffer));
  return buffer;
}

int CWDateTime::getHour()
{
  return String(NTP.getTimeDateString(time(NULL), "%02H")).toInt();
}

int CWDateTime::getMinute()
{
  return String(NTP.getTimeDateString(time(NULL),"%02M")).toInt();
}

int CWDateTime::getSecond()
{
  return String(NTP.getTimeDateString(time(NULL),"%02S")).toInt();
}

int CWDateTime::getDay() 
{
  return String(NTP.getTimeDateString(time(NULL),"%d")).toInt();
}
int CWDateTime::getMonth()
{
  return String(NTP.getTimeDateString(time(NULL),"%02m")).toInt();
}
int CWDateTime::getWeekday() 
{
  return String(NTP.getTimeDateString(time(NULL),"%w")).toInt()-1;
}

long CWDateTime::getMilliseconds() 
{
  return NTP.millis();
}


