/*
  Hotboards_rtcc.cpp - Library to read, write and control the real time clock MCP7941x included in rtc board.
  http://hotboards.org
  adapted and Taken from https://github.com/adafruit/RTClib
  Released into the public domain.
*/
 
#ifndef Hotboards_rtcc_h
#define Hotboards_rtcc_h

#include <inttypes.h>
class TimeSpan;

class DateTime 
{
    public:
        DateTime( uint32_t t = 0 );
        DateTime( uint16_t year, uint8_t month, uint8_t day,
                  uint8_t hour = 0, uint8_t min = 0, uint8_t sec = 0, uint8_t dweek = 0 );
        DateTime( const DateTime& copy );
        DateTime( const char* date, const char* time );
        DateTime( const __FlashStringHelper* date, const __FlashStringHelper* time );
        uint16_t year() const       { return 2000 + yOff; }
        uint8_t month() const       { return m; }
        uint8_t day() const         { return d; }
        uint8_t hour() const        { return hh; }
        uint8_t minute() const      { return mm; }
        uint8_t second() const      { return ss; }
        uint8_t dweek( void ) const { return ss; }
        uint8_t dayOfTheWeek( void ) const;
        // 32-bit times as seconds since 1/1/2000
        uint32_t secondstime( void ) const;  
        // 32-bit times as seconds since 1/1/1970
        uint32_t unixtime( void ) const;
        DateTime operator+(const TimeSpan& span);
        DateTime operator-(const TimeSpan& span);
        TimeSpan operator-(const DateTime& right);

        
    protected:
        uint8_t yOff, m, d, dw, hh, mm, ss;
};

// Timespan which can represent changes in time with seconds accuracy.
class TimeSpan 
{
public:
    TimeSpan (int32_t seconds = 0);
    TimeSpan (int16_t days, int8_t hours, int8_t minutes, int8_t seconds);
    TimeSpan (const TimeSpan& copy);
    int16_t days() const         { return _seconds / 86400L; }
    int8_t  hours() const        { return _seconds / 3600 % 24; }
    int8_t  minutes() const      { return _seconds / 60 % 60; }
    int8_t  seconds() const      { return _seconds % 60; }
    int32_t totalseconds() const { return _seconds; }

    TimeSpan operator+(const TimeSpan& right);
    TimeSpan operator-(const TimeSpan& right);

protected:
    int32_t _seconds;
};

class Hotboards_rtcc 
{
    public:
        uint8_t begin( void );
        void adjust( const DateTime &dt );
        DateTime now( void );
        uint8_t isrunning( void );
        void setVBAT( uint8_t OnOff );
        void setAlarm( const DateTime &dt, uint8_t alarm = 0 );
        uint8_t getAlarmStatus( uint8_t alarm = 0 );
        void clearAlarm( uint8_t alarm = 0 );
        void turnOnAlarm( uint8_t alarm = 0 );
        void turnOffAlarm( uint8_t alarm = 0 );
        
    protected:
        uint8_t on_off = 0;
        
        void stop( void );
        uint8_t readReg( uint8_t address );
        void writeReg( uint8_t address, uint8_t val );
        uint8_t bcd2bin( uint8_t val );
        uint8_t bin2bcd( uint8_t val );

};


#endif
 

