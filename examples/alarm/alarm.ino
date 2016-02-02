/*
  Hotboards_rtcc Library - alarm

 Demonstrates the use a MCP7941x clock calendar.  The Hotboards_rtcc
 library works with this microchip real time clock 
 (http://www.hotboards.org).


 This sketch set an single alarm that will be active 
 after one minute.

  The circuit:
 *  VDD  -->  3.3v
 *  GND  -->  GND
 *  SDA  -->  SDA
 *  SCL  -->  SCL
 
*/
#include <Wire.h>
#include <Hotboards_rtcc.h>

/* days of the week */
const char *week[] = {"Domingo", "Lunes", "Martes", "Miercoles", "Jueves", "Viernes", "Sabado"};
/* months of the year */
const char *months[] = {"ENE","FEB","MAR","ABR","MAY","JUN", "JUL", "AGO","SEP","OCT","NOV","DEC"};

/* lets declare and rtcc instance */
Hotboards_rtcc rtcc;

void setup( void ) 
{
  /* init serial port */
  Serial.begin( 9600 );
  /* this module needs the I2C so we need to init the wire library */
  Wire.begin( );
  /* init the rtcc, just enable the clock if not already enable */
  rtcc.begin( );
  /* set the time when is compiled */
  DateTime time( F(__DATE__), F(__TIME__) );
  rtcc.adjust( time );

  /* Set an alarm after one minute */
  DateTime alarm = time + TimeSpan( 0, 0, 1, 0 );
  rtcc.setAlarm( alarm );
  /* enable the alarm */
  rtcc.turnOnAlarm( );
  
  Serial.println("The alarm will be active in one minute");
}

void loop( void ) 
{
  /* is the alarm active?? */
  if( rtcc.getAlarmStatus( ) == 1 )
  {
      /* clear the alarm */
      rtcc.clearAlarm( );
      /* display the time */
      DateTime time = rtcc.now( );
      Serial.print( "Alarm active at: " );
      display_DateTime( time );
  }
      
}

void display_DateTime( DateTime &dt )
{
  /* using the serial port display the time and date */
  Serial.print( "Time- " );
  Serial.print( dt.hour( ), DEC );

  Serial.print( ":" );
  Serial.print( dt.minute( ), DEC );
  
  Serial.print( ":" );
  Serial.print( dt.second( ), DEC );

  Serial.print( " . Date- " );
  Serial.print( week[dt.dayOfTheWeek( )] );

  Serial.print( "/" );
  Serial.print( dt.day( ), DEC );

  Serial.print( "/" );
  Serial.print( months[dt.month( )] );

  Serial.print( "/" );
  Serial.println( dt.year( ), DEC );
}

