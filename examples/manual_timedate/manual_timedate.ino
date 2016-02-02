/*
  Hotboards_rtcc Library - setting time and date

 Demonstrates the use a MCP7941x clock calendar.  The Hotboards_rtcc
 library works with this microchip real time clock
 (http://www.hotboards.org).


 This sketch set and arbitrary time and date, and then send through
 serial port every five seconds

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
  /* set the time (15:30:00) and date 1/MAR/2001 */
  rtcc.adjust( DateTime( 2001, 2, 1, 15, 30, 0 ) );
}

void loop( void )
{
  /* get the actual time and date */
  DateTime time = rtcc.now( );
  /* display with a nice format */
  display_DateTime( time );
  /* wait only five second */
  delay( 5000 );
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
  Serial.print( week[dt.dweek( )] );

  Serial.print( "/" );
  Serial.print( dt.day( ), DEC );

  Serial.print( "/" );
  Serial.print( months[dt.month( )] );

  Serial.print( "/" );
  Serial.println( dt.year( ), DEC );
}
