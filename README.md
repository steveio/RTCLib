# RTCLib / DateTime
Arduino DateTime class providing simple conversion methods for  Real Time Clock modules.

A fork of https://github.com/NeiroNx/RTCLib

Providing just basic DateTime class conversion methods.

DateTime provides a simple OO method to convert arduino system time constants (__DATE__,__TIME__) to a unix timestamp
  DateTime now = rtc.get();
  now.unixtime();

And other useful time/date manipulation / conversion routines.

RTCLib had been used successfully with DS1307 RTC.

An upgrade to DS3231 RTC required alarm/interupt via SQW pin functions 
implemented in DS3232RTC library ( https://github.com/JChristensen/DS3232RTC ). 
