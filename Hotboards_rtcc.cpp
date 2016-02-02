/*
  Hotboards_rtcc.cpp - Library to read, write and control the real time clock MCP7941x included in rtc board.
  http://hotboards.org
  adapted and taken from https://github.com/adafruit/RTClib
  Released into the public domain.
*/

#include <Wire.h>
#include "Hotboards_rtcc.h"
#ifdef __AVR__
 #include <avr/pgmspace.h>
#elif defined(ESP8266)
 #include <pgmspace.h>
#elif defined(ARDUINO_ARCH_SAMD)
// nothing special needed
#elif defined(ARDUINO_SAM_DUE)
 #define PROGMEM
 #define pgm_read_byte(addr) (*(const unsigned char *)(addr))
 #define Wire Wire1
#endif

#define RTC_ADDR          (uint8_t)(0xDE >> 1)
#define EEPROM_ADDR       (uint8_t)(0xAE >> 1)
#define RTC_STARTADDR     (uint8_t)0x00
#define ALARM_STARTADDR   (uint8_t)0x0A
#define CTRL_STARTADDR    (uint8_t)0x07
#define SRAM_SARTADDR     (uint8_t)0x20
#define EEPROM_SARTADDR   (uint8_t)0x00
#define PEEPROM_SARTADDR  (uint8_t)0xF0

#define SECONDS_FROM_1970_TO_2000 946684800

const uint8_t daysInMonth[] PROGMEM = { 31,28,31,30,31,30,31,31,30,31,30,31 };

// number of days since 2000/01/01, valid for 2001..2099
static uint16_t date2days(uint16_t y, uint8_t m, uint8_t d) 
{
    if (y >= 2000)
        y -= 2000;
    uint16_t days = d;
    for (uint8_t i = 1; i < m; ++i)
        days += pgm_read_byte(daysInMonth + i - 1);
    if (m > 2 && y % 4 == 0)
        ++days;
    return days + 365 * y + (y + 3) / 4 - 1;
}

static long time2long(uint16_t days, uint8_t h, uint8_t m, uint8_t s) 
{
    return ((days * 24L + h) * 60 + m) * 60 + s;
}

static uint8_t conv2d(const char* p) 
{
    uint8_t v = 0;
    if ('0' <= *p && *p <= '9')
        v = *p - '0';
    return 10 * v + *++p - '0';
}

/*
 * Constructor that use time in a 32 bit variable
 */
DateTime::DateTime( uint32_t t )
{
    t -= SECONDS_FROM_1970_TO_2000;    // bring to 2000 timestamp from 1970
    ss = t % 60;
    t /= 60;
    mm = t % 60;
    t /= 60;
    hh = t % 24;
    uint16_t days = t / 24;
    uint8_t leap;
    for( yOff = 0 ; ; ++yOff ) 
    {
        leap = yOff % 4 == 0;
        if (days < 365 + leap)
        {
            break;
        }
        days -= 365 + leap;
    }
    for (m = 1; ; ++m) 
    {
        uint8_t daysPerMonth = pgm_read_byte(daysInMonth + m - 1);
        if (leap && m == 2)
        {
            ++daysPerMonth;
        }
        if (days < daysPerMonth)
        {
            break;
        }
        days -= daysPerMonth;
    }
    d = days + 1;
}
        
/*
 * Constructor that use time variables for each element in decimal
 */
DateTime::DateTime( uint16_t year, uint8_t month, uint8_t day,
                  uint8_t hour, uint8_t min, uint8_t sec, uint8_t dweek )
{
    if( year >= 2000 )
    {
        year -= 2000;
    }
    yOff = year;
    m = month;
    d = day;
    hh = hour;
    mm = min;
    ss = sec;
    dw = dweek;
}
        
/*
 * Constructor tcreate a copy of DateTime object
 */
DateTime::DateTime (const DateTime& copy):
  yOff(copy.yOff),
  m(copy.m),
  d(copy.d),
  hh(copy.hh),
  mm(copy.mm),
  ss(copy.ss)
{}


/*
 * A convenient constructor for using "the compiler's time":
   DateTime now (__DATE__, __TIME__);
 */
DateTime::DateTime( const char* date, const char* time ) 
{
    // sample input: date = "Dec 26 2009", time = "12:34:56"
    yOff = conv2d(date + 9);
    // Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec 
    switch( date[0] ) 
    {
        case 'J': m = date[1] == 'a' ? 1 : m = date[2] == 'n' ? 6 : 7; break;
        case 'F': m = 2; break;
        case 'A': m = date[2] == 'r' ? 4 : 8; break;
        case 'M': m = date[2] == 'r' ? 3 : 5; break;
        case 'S': m = 9; break;
        case 'O': m = 10; break;
        case 'N': m = 11; break;
        case 'D': m = 12; break;
    }
    d  = conv2d( date + 4 );
    hh = conv2d( time );
    mm = conv2d( time + 3 );
    ss = conv2d( time + 6 );
}

/*
 * A convenient constructor for using "the compiler's time":
 * This version will save RAM by using PROGMEM to store it by using the F macro.
 * DateTime now (F(__DATE__), F(__TIME__));
*/
DateTime::DateTime( const __FlashStringHelper* date, const __FlashStringHelper* time ) 
{
    // sample input: date = "Dec 26 2009", time = "12:34:56"
    char buff[11];
    memcpy_P(buff, date, 11);
    yOff = conv2d(buff + 9);
    // Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec
    switch( buff[0] ) 
    {
        case 'J': m = buff[1] == 'a' ? 1 : m = buff[2] == 'n' ? 6 : 7; break;
        case 'F': m = 2; break;
        case 'A': m = buff[2] == 'r' ? 4 : 8; break;
        case 'M': m = buff[2] == 'r' ? 3 : 5; break;
        case 'S': m = 9; break;
        case 'O': m = 10; break;
        case 'N': m = 11; break;
        case 'D': m = 12; break;
    }
    d = conv2d(buff + 4);
    memcpy_P(buff, time, 8);
    hh = conv2d(buff);
    mm = conv2d(buff + 3);
    ss = conv2d(buff + 6);
}

uint8_t DateTime::dayOfTheWeek( void ) const
{    
    uint16_t day = date2days( yOff, m, d );
    
    return ( day + 6 ) % 7; // Jan 1, 2000 is a Saturday, i.e. returns 6
}

uint32_t DateTime::unixtime( void ) const
{
  uint32_t t;
  uint16_t days = date2days( yOff, m, d );
  
  t = time2long(days, hh, mm, ss);
  t += SECONDS_FROM_1970_TO_2000;  // seconds from 1970 to 2000

  return t;
}

uint32_t DateTime::secondstime( void ) const
{
  uint32_t t;
  uint16_t days = date2days( yOff, m, d );
  
  t = time2long( days, hh, mm, ss );
  return t;
}




DateTime DateTime::operator+(const TimeSpan& span) 
{
  return DateTime(unixtime()+span.totalseconds());
}

DateTime DateTime::operator-(const TimeSpan& span) 
{
  return DateTime(unixtime()-span.totalseconds());
}

TimeSpan DateTime::operator-(const DateTime& right) 
{
  return TimeSpan(unixtime()-right.unixtime());
}



TimeSpan::TimeSpan (int32_t seconds):
  _seconds(seconds)
{}

TimeSpan::TimeSpan (int16_t days, int8_t hours, int8_t minutes, int8_t seconds):
  _seconds((int32_t)days*86400L + (int32_t)hours*3600 + (int32_t)minutes*60 + seconds)
{}

TimeSpan::TimeSpan (const TimeSpan& copy):
  _seconds(copy._seconds)
{}

TimeSpan TimeSpan::operator+(const TimeSpan& right) 
{
  return TimeSpan(_seconds+right._seconds);
}

TimeSpan TimeSpan::operator-(const TimeSpan& right) 
{
  return TimeSpan(_seconds-right._seconds);
}




/*
 * enable internal oscilator if this is disable (start the clock)
 */
uint8_t Hotboards_rtcc::begin( void )
{
    if( isrunning( ) == 0 )
    {
        writeReg( RTC_STARTADDR, 0x80 );
    }
}

/*
 * set a new time and date
 */
void Hotboards_rtcc::adjust( const DateTime &dt )
{
    stop();
    Wire.beginTransmission( RTC_ADDR );
    Wire.write( RTC_STARTADDR ); // start at location 0
    Wire.write( bin2bcd( dt.second( ) ) | 0x80 );
    Wire.write( bin2bcd( dt.minute( ) ) );
    Wire.write( bin2bcd( dt.hour( ) ) );
    Wire.write( dt.dayOfTheWeek( ) | on_off ); // day of the week
    Wire.write( bin2bcd( dt.day( ) ) );
    Wire.write( bin2bcd( dt.month( ) ) );
    Wire.write( bin2bcd( dt.year( ) - 2000 ) );
    Wire.endTransmission( );
}

/*
 * return an DateTime object with the actual time and date
 */
DateTime Hotboards_rtcc::now( void )
{
    Wire.beginTransmission( RTC_ADDR );
    Wire.write( RTC_STARTADDR ); // start at location 0
    Wire.endTransmission( );
  
    Wire.requestFrom( RTC_ADDR, (uint8_t)7 );
    uint8_t ss = bcd2bin( Wire.read( ) & 0x7F );
    uint8_t mm = bcd2bin( Wire.read( ) );
    uint8_t hh = bcd2bin( Wire.read( ) );
    uint8_t dw = Wire.read( ) & 0x07;
    uint8_t d  = bcd2bin( Wire.read( ) );
    uint8_t m  = bcd2bin( Wire.read( ) & 0xDF );
    uint16_t y = bcd2bin( Wire.read( ) ) + 2000;
    
    return DateTime( y, m, d, hh, mm, ss, dw );  
}

/*
 * return a true if the rtcc is running
 */
uint8_t Hotboards_rtcc::isrunning( void )
{
    uint8_t running = readReg( RTC_STARTADDR + 3 ) & 0x20;
    return running >> 5;
}

/*
 * stop the internal rtcc clock
 */
void Hotboards_rtcc::stop( void )
{
   writeReg( RTC_STARTADDR, 0x00 );
   while( isrunning( ) == 1 ); 
}

void Hotboards_rtcc::setVBAT( uint8_t OnOff )
{
    on_off = ( OnOff & 0x01 ) << 3;
}

void Hotboards_rtcc::setAlarm( const DateTime &dt, uint8_t alarm )
{
    Wire.beginTransmission( RTC_ADDR );
    Wire.write( ALARM_STARTADDR ); 
    Wire.write( bin2bcd( dt.second( ) ) );
    Wire.write( bin2bcd( dt.minute( ) ) );
    Wire.write( bin2bcd( dt.hour( ) ) );
    Wire.write( dt.dayOfTheWeek( ) | 0x70 );
    Wire.write( bin2bcd( dt.day( ) ) );
    Wire.write( bin2bcd( dt.month( ) ) );
    Wire.write( bin2bcd( dt.year( ) - 2000 ) );
    Wire.endTransmission( );    
}

uint8_t Hotboards_rtcc::getAlarmStatus( uint8_t alarm )
{
    uint8_t status = readReg( ALARM_STARTADDR + 3 );
    return ( status >> 3 ) & 0x01;
}

void Hotboards_rtcc::clearAlarm( uint8_t alarm )
{
    uint8_t status = readReg( ALARM_STARTADDR + 3 );
    writeReg( ALARM_STARTADDR + 3, (status  & 0xF7) );
}

void Hotboards_rtcc::turnOnAlarm( uint8_t alarm )
{
    uint8_t ctrl = readReg( CTRL_STARTADDR );
    writeReg( CTRL_STARTADDR, ctrl | 0x10 );
}

void Hotboards_rtcc::turnOffAlarm( uint8_t alarm )
{
    uint8_t ctrl = readReg( CTRL_STARTADDR );
    writeReg( CTRL_STARTADDR, ctrl & 0xEF );
}

uint8_t Hotboards_rtcc::readReg( uint8_t address )
{
    Wire.beginTransmission( RTC_ADDR );
    Wire.write( address ); // start at location 0
    Wire.endTransmission( );
  
    Wire.requestFrom( RTC_ADDR, (uint8_t)1 );
    return Wire.read( );
}

void Hotboards_rtcc::writeReg( uint8_t address, uint8_t val )
{
   Wire.beginTransmission( RTC_ADDR );
   Wire.write( address );
   Wire.write( val );
   Wire.endTransmission( ); 
}
      
uint8_t Hotboards_rtcc::bcd2bin( uint8_t val ) 
{ 
    return val - 6 * ( val >> 4 ); 
}

uint8_t Hotboards_rtcc::bin2bcd( uint8_t val ) 
{ 
    return val + 6 * ( val / 10 ); 

}


