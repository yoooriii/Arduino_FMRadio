// Programm: fm_radio_module_em5807m_rev100.ino
// Language: Arduinio C++
// Hardware: Arduino Nano 3.0 + EM5807M module with RDA5807M Chip.
// Version:  1.00
// Autor:    www.5v.ru
// Link:     www.5v.ru/start/em5807m-arduino-nano-30.htm

/*
  LiquidCrystal Library - Hello World

 Demonstrates the use a 16x2 LCD display.  The LiquidCrystal
 library works with all LCD displays that are compatible with the
 Hitachi HD44780 driver. There are many of them out there, and you
 can usually tell them by the 16-pin interface.

 This sketch prints "Hello World!" to the LCD
 and shows the time.

  The circuit:
 * LCD RS pin to digital pin 12
 * LCD Enable pin to digital pin 11
 * LCD D4 pin to digital pin 5
 * LCD D5 pin to digital pin 4
 * LCD D6 pin to digital pin 3
 * LCD D7 pin to digital pin 2
 * LCD R/W pin to ground
 * LCD VSS pin to ground
 * LCD VCC pin to 5V
 * 10K resistor:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)

 Library originally added 18 Apr 2008
 by David A. Mellis
 library modified 5 Jul 2009
 by Limor Fried (http://www.ladyada.net)
 example added 9 Jul 2009
 by Tom Igoe
 modified 22 Nov 2010
 by Tom Igoe

 This example code is in the public domain.

 http://www.arduino.cc/en/Tutorial/LiquidCrystal
 */

#include <LiquidCrystal.h>
#include <Wire.h>               // I2C-Library
#include "radoistations_kiev.h"

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

int scanKeypad(int delay);
void selectRadiostationAtIndex(const short radio_index);
short channel = 0;

void setup()                    // Инициализация.
{
  	Wire.begin();                 // Инициализация двухпроводной шины I2C.


  	// set up the LCD's number of columns and rows:
  	lcd.begin(16, 2);
  	// Print a message to the LCD.
  	lcd.print("hello, world!");

  	pinMode(6, OUTPUT);
  	digitalWrite(6, HIGH);
	for (int pin_in=7; pin_in<=10; ++pin_in) {
   		pinMode(pin_in, INPUT);
//  		digitalWrite(pin_in, HIGH);
	}

	selectRadiostationAtIndex(0);
}

typedef enum KeyCodes {
	KeyCodeUp = 2,
	KeyCodeDown = 1,
	KeyCodeOn = 4,
	KeyCodeOff = 8
} KeyCodes;

int key_code = 0;

void loop()
{
	const int key = scanKeypad(200);
	if (key != key_code) {
		key_code = key;
		short ch = channel;
		//	check what keys are down
		if (key_code & KeyCodeUp) {
			++ch;
		}
		if (key_code & KeyCodeDown) {
			--ch;
		}
		//	validate
		if (ch < 0) {
			ch = 0;
		}
		else if (ch >= radiostations_count) {
			ch >= radiostations_count - 1;
		}
		//	select new channel if changed
		if (ch != channel) {
			channel = ch;
			selectRadiostationAtIndex(channel);
		}
		//	display on/off
		if (key_code & KeyCodeOn) {
				digitalWrite(6, HIGH);
		}
		if (key_code & KeyCodeOff) {
				digitalWrite(6, LOW);
		}
	}
}

void selectRadiostationAtIndex(const short radio_index) {
	const Radiostation *radio = &radiostations[radio_index];
	char print_string[17];
	const unsigned short str_leng = 16;
	unsigned short i;

	int frq = round(radio->f*100.0);
	int frqH = frq/100;
	int frqL = frq - frqH * 100;

	i = snprintf(print_string, 16, "#%02d %02d.%02dMHz", (int)radio_index, frqH, frqL);
	for (; i<str_leng; ++i) {
		print_string[i] = ' ';
	}
	print_string[str_leng] = 0;
	lcd.setCursor(0, 0);
	lcd.print(print_string);

	for (i=0; i<str_leng && radio->name[i]; ++i) {
		print_string[i] = radio->name[i];
	}
	for (; i<str_leng; ++i) {
		print_string[i] = ' ';
	}
	print_string[str_leng] = 0;
	// set the cursor to column 0, line 1
	// (note: line 1 is the second row, since counting begins with 0):
	lcd.setCursor(0, 1);
	lcd.print(print_string);

	setFrequency(radio->f);
}

int scanKeypad(int time) {
	const int key_delay = 20;
	const int loop_count = time/key_delay;

	int key_value = 0;
	for (int iloop=0; iloop<loop_count; ++iloop) {
		for (int pin_in=7; pin_in<=10; ++pin_in) {
			int key = digitalRead(pin_in);
			if (0 == key) {
				key = 1 << (pin_in - 7);
				key_value = key_value | key;
			}

		}
		if (key_value) {
			delay(100);
			break;
		}
		delay(key_delay);
	}
	return key_value;
}


void setFrequency(float fmhz)  // Функция загрузки частоты настройки в модуль EM5807M.
{
  const int  frequencyB = 4.0 * ((double)fmhz * 1000000.0 + 225000.0) / 32768.0;
  char frequencyH = frequencyB >> 8;     // Старший байт.
  char frequencyL = frequencyB & 0XFF;   // Накладываем маску 0xFF на младший байт.
  Wire.beginTransmission(0x60);          // Адрес чипа RDA5807M
  Wire.write(frequencyH);                // Старший байт.
  Wire.write(frequencyL);                // Младший байт.
  Wire.write(0xB8);                      // 1011 1000     =Стерео
  Wire.write(0x10);                      // 0001 0000
  Wire.write((byte)0x00);                //
  Wire.endTransmission();                // формируем I2C-Stop.
}


