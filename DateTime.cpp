// Code by JeeLabs http://news.jeelabs.org/code/
// Released to the public domain! Enjoy!

#include "DateTime.h"

#ifdef __AVR__
 #include <avr/pgmspace.h>
#elif defined(ESP8266) 
 #include <pgmspace.h> 
#else
 #define PROGMEM
 #define pgm_read_byte(addr) (*(const unsigned char *)(addr))
#endif

#define SECONDS_PER_DAY 86400L
#define SECONDS_FROM_1970_TO_2000 946684800


////////////////////////////////////////////////////////////////////////////////
// utility code, some of this could be exposed in the DateTime API if needed
static uint8_t bcd2bin (uint8_t val) { return val - 6 * (val >> 4); }
static uint8_t bin2bcd (uint8_t val) { return val + 6 * (val / 10); }

const uint8_t daysInMonth[] PROGMEM = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

// number of days since 2000/01/01, valid for 2001..2099
static uint16_t date2days(uint16_t y, uint8_t m, uint8_t d) {
    if (y >= 2000)
        y -= 2000;
    uint16_t days = d;
    for (uint8_t i = 1; i < m; ++i)
        days += pgm_read_byte(daysInMonth + i - 1);
    if (m > 2 && y % 4 == 0)
        ++days;
    return days + 365 * y + (y + 3) / 4 - 1;
}

static long time2long(uint16_t days, uint8_t h, uint8_t m, uint8_t s) {
    return ((days * 24L + h) * 60 + m) * 60 + s;
}

static uint8_t conv2d(const char* p) {
    uint8_t v = 0;
    if ('0' <= *p && *p <= '9')
        v = *p - '0';
    return 10 * v + *++p - '0';
}

////////////////////////////////////////////////////////////////////////////////
// DateTime implementation - ignores time zones and DST changes
// NOTE: also ignores leap seconds, see http://en.wikipedia.org/wiki/Leap_second
DateTime::DateTime (uint32_t t) {
    t -= SECONDS_FROM_1970_TO_2000; // bring to 2000 timestamp from 1970

    ss = t % 60;
    t /= 60;
    mm = t % 60;
    t /= 60;
    hh = t % 24;
    uint16_t days = t / 24;
    uint8_t leap;
    for (yOff = 0; ;++yOff) {
        leap = yOff % 4 == 0;
        if (days < 365 + leap)
            break;
        days -= 365 + leap;
    }

    for (m = 1; ; ++m) {
        uint8_t daysPerMonth = pgm_read_byte(daysInMonth + m - 1);
        if (leap && m == 2)
            ++daysPerMonth;
        if (days < daysPerMonth)
            break;
        days -= daysPerMonth;
    }

    d = days + 1;
}

DateTime::DateTime (uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t min, uint8_t sec) {
    if (year >= 2000)
        year -= 2000;
    yOff = year;
    m = month;
    d = day;
    hh = hour;
    mm = min;
    ss = sec;
}

// A convenient constructor for using "the compiler's time":
//   DateTime now(__DATE__, __TIME__);
// NOTE: using PSTR would further reduce the RAM footprint
DateTime::DateTime (const char* date, const char* time) {
    SetDate(date);
    SetTime(time);
}

// A convenient constructor for using "the compiler's time":
// This version will save RAM by using PROGMEM to store it by using the F macro.
// DateTime now(F(__DATE__), F(__TIME__));
DateTime::DateTime (const __FlashStringHelper* date, const __FlashStringHelper* time) {
    // sample input: date = "Dec 26 2009", time = "12:34:56"
    char _date[11];
    char _time[8];
    memcpy_P(_date, date, 11);
    memcpy_P(_time, time, 8);
    DateTime(date, time);
}

DateTime::DateTime(const char* sdate) {
    DateTime(sdate, sdate + 11);
}

/*********************************************/
/*  Comparison                               */
/*********************************************/

unsigned char DateTime::equals(const char* sdate) const {
    return equals(DateTime(sdate));
}

unsigned char DateTime::equals(const DateTime &date) const {
    return yOff == date.year() - 2000
        && m == date.month()
        && d == date.day()
        && hh == date.hour()
        && mm == date.minute()
        && ss == date.second();
}

unsigned char DateTime::operator<(const DateTime &date) const{
    return unixtime() < date.unixtime();
}

unsigned char DateTime::operator>(const DateTime &date) const{
    return unixtime() > date.unixtime();
}

unsigned char DateTime::operator<=(const DateTime &date) const{
    return unixtime() <= date.unixtime();
}

unsigned char DateTime::operator>=(const DateTime &date) const{
    return unixtime() >= date.unixtime();
}

uint8_t DateTime::dayOfTheWeek() const {
    uint16_t day = date2days(yOff, m, d);
    return (day + 6) % 7; // Jan 1, 2000 is a Saturday, i.e. returns 6
}

uint32_t DateTime::unixtime() const {
    uint32_t t;
    uint16_t days = date2days(yOff, m, d);
    t = time2long(days, hh, mm, ss);
    t += SECONDS_FROM_1970_TO_2000; // seconds from 1970 to 2000

    return t;
}

void DateTime::SetTime(const char* time) {
    hh = conv2d(time);
    mm = conv2d(time + 3);
    ss = conv2d(time + 6);
}

void DateTime::SetDate(const char* date) {
    // sample input: date = "Dec 26 2009", time = "12:34:56"
    // or date = "26-12-2009"
    yOff = conv2d(date + 9);
    // Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec
    d = conv2d(date + 4);
    switch (date[0]) {
        case 'J': m = date[1] == 'a' ? 1 : m = date[2] == 'n' ? 6 : 7; break;
        case 'F': m = 2; break;
        case 'A': m = date[2] == 'r' ? 4 : 8; break;
        case 'M': m = date[2] == 'r' ? 3 : 5; break;
        case 'S': m = 9; break;
        case 'O': m = 10; break;
        case 'N': m = 11; break;
        case 'D': m = 12; break;
        default:
            yOff = conv2d(date + 8);
            d = conv2d(date);
            m = conv2d(date + 3);
    }
}

/*
DateTime DateTime::operator+ (const TimeDelta& delta) const {
    return DateTime(unixtime() + delta.totalseconds());
}

DateTime DateTime::operator- (const TimeDelta& delta) const {
    return DateTime(unixtime() - delta.totalseconds());
}

TimeDelta DateTime::operator- (const DateTime& right) const {
    return TimeDelta(unixtime() - right.unixtime());
}
*/

char* DateTime::format(char* ret) {
    for (int i = 0; i < strlen(ret); i++) {
        if (ret[i] == 'h' && ret[i + 1] == 'h') {
            ret[i] = '0' + hh / 10;
            ret[i + 1] = '0' + hh % 10;
        }

        if (ret[i] == 'm' && ret[i + 1] == 'm') {
            ret[i] = '0' + mm / 10;
            ret[i + 1] = '0' + mm % 10;
        }

        if (ret[i] == 's' && ret[i + 1] == 's') {
            ret[i] = '0' + ss / 10;
            ret[i + 1] = '0' + ss % 10;
        }

        if (ret[i] == 'D' && ret[i + 1] == 'D') {
            ret[i] = '0' + d / 10;
            ret[i + 1] = '0' + d % 10;
        }

        if (ret[i] == 'M' && ret[i + 1] == 'M') {
            ret[i] = '0' + m / 10;
            ret[i + 1] = '0' + m % 10;
        }

        if (ret[i] == 'Y') {
            if (ret[i + 3] == 'Y') {
                ret[i] = '2';
                ret[i + 1] = '0';
                ret[i + 2] = '0' + (yOff / 10) % 10;
                ret[i + 3] = '0' + yOff % 10;
            }
            else if (ret[i + 1] == 'Y') {
                ret[i] = '0' + (yOff / 10) % 10;
                ret[i + 1] = '0' + yOff % 10;
            }
        }
    }
    return ret;
}

char* DateTime::toStr() {
    // Format: "YYYY/MM/DD hh:mm:ss"
    static char str[20] = "20";
    uint8_t t;
    for (uint8_t i = 2; i < 19; i++) {
        switch (i) {
            case 4:
            case 7: str[i] = '/'; break;
            case 10: str[i] = ' '; break;
            case 13:
            case 16: str[i] = ':'; break;
            case 2: t = yOff; break;
            case 5: t = m; break;
            case 8: t = d; break;
            case 11: t = hh; break;
            case 14: t = mm; break;
            case 17: t = ss; break;
            default: str[i - 1] = '0' + t / 10; str[i] = '0' + t % 10;
        }
    }

    return str;
}

