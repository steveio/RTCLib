// Code by JeeLabs http://news.jeelabs.org/code/
// Released to the public domain! Enjoy!

#ifndef _DTLIB_H_
#define _DTLIB_H_

#if (ARDUINO >= 100)
 #include <Arduino.h> // capital A so it is error prone on case-sensitive filesystems
#else
 #include <WProgram.h>
#endif


// Simple general-purpose date/time class (no TZ / DST / leap second handling!)
class DateTime {
    public:
        char* format(char* ret);
        char* toStr();
        DateTime(uint32_t t =0);
        DateTime(uint16_t year, uint8_t month, uint8_t day,
                 uint8_t hour =0, uint8_t min =0, uint8_t sec =0);
        DateTime(const char* date, const char* time);
        DateTime(const __FlashStringHelper* date, const __FlashStringHelper* time);
        DateTime(const char* sdate);
        uint16_t year() const { return 2000 + yOff; }
        uint8_t month() const { return m; }
        uint8_t day() const { return d; }
        uint8_t hour() const { return hh; }
        uint8_t minute() const { return mm; }
        uint8_t second() const { return ss; }
        uint8_t dayOfTheWeek() const;
        void setyear(uint16_t year) { yOff = year - 2000; }
        void setmonth(uint8_t month) { m = month; }
        void setday(uint8_t day) { d = day; }
        void sethour(uint8_t hour) { hh = hour%24; }
        void setminute(uint8_t minute) { mm = minute%60; }
        void setsecond(uint8_t second) { ss = second%60; }
        void SetTime(const char* time);
        void SetDate(const char* date);
        // 32-bit times as seconds since 1/1/2000
        long secondstime() const;
        // 32-bit times as seconds since 1/1/1970
        uint32_t unixtime() const;
        unsigned char equals(const char* sdate) const; // DD-MM-YYYY hh:mm:ss
        unsigned char equals(const DateTime &date) const;
        unsigned char operator== (const char *sdate) const { return equals(sdate); }
        unsigned char operator== (const DateTime &date) const { return equals(date); }
        unsigned char operator!= (const char *sdate) const { return !equals(sdate); }
        unsigned char operator!= (const DateTime &date) const { return !equals(date); }
        unsigned char operator< (const DateTime &date) const;
        unsigned char operator> (const DateTime &date) const;
        unsigned char operator<= (const DateTime &date) const;
        unsigned char operator>= (const DateTime &date) const;

        //DateTime operator+ (const TimeDelta& delta) const;
        //DateTime operator- (const TimeDelta& delta) const;
        //TimeDelta operator- (const DateTime& right) const;

    protected:
        uint8_t yOff, m, d, hh, mm, ss;
};

#endif 
