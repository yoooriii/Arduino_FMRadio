#include "projectprefix.h"
#include <LiquidCrystal.h>
#include <Wire.h>

#include "RADIO_rda5807m.h"
#include "RDSParser.h"

//	under construction...
#include <OneButton2.h>
OneButton2 button0(A0, true);
OneButton2 button1(A1, true);

#include "radoistations_kiev.h"

#include <RotaryEncoder.h>



// Setup a RoraryEncoder for pins A2 and A3:
// RotaryEncoder encoder(A2, A3);


// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);


RADIO_RDA5807M radio;
static short rindex = 0;
/// get a RDS parser
RDSParser2 rds;


void actGoUp(OneButton2 *sender) {
	LOGTHIS;
	rindex = selectChannel(rindex + 1);
}

void actGoDown(OneButton2 *sender) {
	LOGTHIS;
	rindex = selectChannel(rindex - 1);
}

void actLong0(OneButton2 *sender) {
	LOGTHIS;
	uint8_t vol = radio.getVolume();
	if (vol < 15) {
		radio.setVolume(vol+1);
	}
}

void actLong1(OneButton2 *sender) {
	LOGTHIS;
	uint8_t vol = radio.getVolume();
	if (vol > 1) {
		radio.setVolume(vol-1);
	}
}

void actDoubleClick0(OneButton2 *sender) {
	LOGTHIS;
}

void actDoubleClick1(OneButton2 *sender) {
	LOGTHIS;
}

#pragma mark -

void setup()
{
	Serial.begin(9600);
	Wire.begin();

	delay(100);

	//	turn lcd led backlight on
  	pinMode(6, OUTPUT);
  	digitalWrite(6, HIGH);


	Serial.println(";");
 	Serial.print("main setup; btn.sz:");
 	Serial.print((int)sizeof(OneButton2));
	Serial.print(" rot.sz:");
	Serial.print((int)sizeof(RotaryEncoder));
	Serial.print(" radio.sz:");
	Serial.print((int)sizeof(RADIO_RDA5807M));
	Serial.print(" rds.sz:");
	Serial.print((int)sizeof(RDSParser2));
	Serial.print(" lcd.sz:");
	Serial.print((int)sizeof(LiquidCrystal));
	Serial.print(" list.sz:");
	Serial.print((int)sizeof(Radiostation));
	Serial.print("x");
	Serial.print((int)radiostations_count);
	Serial.print(" float:");
	Serial.print((int)sizeof(float));
	Serial.print(" short:");
	Serial.print((int)sizeof(short));
	Serial.println(";");
	//	test:
	DisplayTime(500);

	// RotaryEncoder: You may have to modify the next 2 lines if using other pins than A2 and A3
// 	PCICR |= (1 << PCIE1);    // This enables Pin Change Interrupt 1 that covers the Analog input pins or Port C.
// 	PCMSK1 |= (1 << PCINT10) | (1 << PCINT11);  // This enables the interrupt for pin 2 and 3 of Port C.

	radio.init();
	radio.debugEnable(false);
	delay(100);
	radio.setBandFrequency(RADIO_BAND_FM, 10360);
	delay(100);
	radio.setMono(false);
	radio.setMute(false);
	radio.setVolume(15);

  	// set up the LCD's number of columns and rows:
  	lcd.begin(16, 2);
  	// Print a message to the LCD.
  	lcd.print("hello, world!");


	button0.attachClick(actGoUp);
	button0.attachDoubleClick(actDoubleClick0);
	button0.attachDuringLongPress(actLong0);
	button1.attachClick(actGoDown);
	button1.attachDoubleClick(actDoubleClick1);
	button1.attachDuringLongPress(actLong1);



  	//	configure keyboard input
// 	for (int pin_in=7; pin_in<=10; ++pin_in) {
//    		pinMode(pin_in, INPUT);
// 	}


	// setup RDS
 	rds.attachServicenNameCallback(DisplayServiceName);
 	rds.attachTimeCallback(DisplayTime);
 	rds.attachTextCallback(RDSTextDidUpdate);
 	radio.attachReceiveRDS(RDS_process);
}

extern Radiostation radiostations[];
extern const short radiostations_count;
const int TextBufferLength = 17;
char TextBuffer[TextBufferLength];
static int currentPos = 0;
static short rdstxtOffset = 0;

void blinkDisplay() {
	static	unsigned long updateTime = 0;
	static	bool displayON = false;

	const unsigned long now = millis(); // current (relative) time in msecs.
	if (0 == updateTime) {
		updateTime = now;
		digitalWrite(6, HIGH);
		displayON = true;
		return;
	}

	const unsigned long dt = now - updateTime;
	const unsigned long maxPeriod = displayON ? 500 : 50;
	if (dt > maxPeriod) {
		updateTime = now;
		displayON = !displayON;
		digitalWrite(6, displayON ? HIGH:LOW);
	}
}

void updateRDSText() {
	static	unsigned long updateTime = 0;
	const unsigned long now = millis(); // current (relative) time in msecs.
	if (0 == updateTime) {
		updateTime = now;
		return;
	}

	const unsigned long dt = now - updateTime;
	if (dt > 300) {
		updateTime = now;
		const char * rdstxt = rds.getRDSText();
		const short len = strlen(rdstxt);
		if (rdstxtOffset + 16 + 1 < len) {
			++rdstxtOffset;
		}
		else {
			rdstxtOffset = 0;
		}
		DisplayText(rdstxt, rdstxtOffset);
	}
}

void loop()
{
	blinkDisplay();
	updateRDSText();

// 	const int newPos = encoder.getPosition();
// 	if (currentPos != newPos) {
// 		Serial.print(newPos);
// 		Serial.println();
// 		currentPos = newPos;
//
// 		rindex = currentPos;
// 		//	select another radiostation
// 		Radiostation *r = &radiostations[rindex];
// 		float f = r->f;
// 		const uint16_t fint = round(100.0*f);
// 		radio.setFrequency(fint);
//
// 		snprintf(TextBuffer, sizeof(TextBuffer), "%d, %d", (int)rindex, (int)fint);
// 		Serial.println(TextBuffer);
// 	}

	button0.tick();
	button1.tick();
	// check for RDS data
	radio.checkRDS();


// 	Serial.println(__PRETTY_FUNCTION__);
	delay(20);

}

// RotaryEncoder: The Interrupt Service Routine for Pin Change Interrupt 1
// This routine will only be called on any signal change on A2 and A3: exactly where we need to check.
// ISR(PCINT1_vect) {
// 	LOGTHIS;
// 	encoder.tick(); // just call tick() to check the state.
// //	updateEncoder();
// }

// void updateEncoder () {
// 	static int pos = 0;
//
// 	const int newPos = encoder.getPosition();
// 	if (pos != newPos) {
// 		Serial.print(newPos);
// 		Serial.println();
//
// // 	if (newPos >= radiostations_count) {
// // 		pos = 0;
// // 		encoder.setPosition(pos);
// // 	}
// // 	else if (newPos < 0) {
// // 		pos = radiostations_count - 1;
// // 		encoder.setPosition(pos);
// // 	}
// // 	else {
// // 	    pos = newPos;
// // 	}
//
//     pos = newPos;
//     rindex = pos;
//
//   } // if
// }

short selectChannel(short channel) {
	if (channel >= radiostations_count) {
		channel = 0;
	}
	else if (channel < 0) {
		channel = radiostations_count - 1;
	}

	rdstxtOffset = 0;
	rds.reset();

	if (channel != rindex) {
		//	select another radiostation
		Radiostation *station = &radiostations[channel];
		const uint16_t fint = station->f;
		radio.setFrequency(fint);

		const int str_leng = TextBufferLength;
		snprintf(TextBuffer, str_leng, "%d, %d", (int)channel, (int)fint);
		Serial.println(TextBuffer);
		//
		const int frqH = fint/100;
		const int frqL = fint - 100 * frqH;
		int i = snprintf(TextBuffer, 16, "#%02d %02d.%02dMHz", (int)channel, frqH, frqL);
		for (; i<str_leng; ++i) {
			TextBuffer[i] = ' ';
		}
		TextBuffer[str_leng] = 0;
		lcd.setCursor(0, 0);
		lcd.print(TextBuffer);

		for (i=0; i<str_leng && station->name[i]; ++i) {
			TextBuffer[i] = station->name[i];
		}
		for (; i<str_leng; ++i) {
			TextBuffer[i] = ' ';
		}
		TextBuffer[str_leng] = 0;
		// set the cursor to column 0, line 1
		// (note: line 1 is the second row, since counting begins with 0):
		lcd.setCursor(0, 1);
		lcd.print(TextBuffer);
	}
	return channel;
}

#pragma mark -
#pragma mark RDS logic

void RDS_process(const uint16_t *blocks) {
 	rds.processData(blocks);
}

/// Update the ServiceName text on the LCD display when in RDS mode.
void DisplayServiceName(const char *name)
{
	Serial.print("RDS:"); Serial.println(name);

// 	if (rot_state == STATE_FREQ)
	{
		int ibuf;
		for (ibuf=0; ibuf < TextBufferLength-1 && TextBuffer[ibuf]; ++ibuf) {
			TextBuffer[ibuf] = name[ibuf];
		}
		for (; ibuf < TextBufferLength-1; ++ibuf) {
			TextBuffer[ibuf] = ' ';
		}
		TextBuffer[TextBufferLength-1] = 0;
		//
		lcd.setCursor(0, 1);
		lcd.print(TextBuffer);
	}
} // DisplayServiceName()

void RDSTextDidUpdate(const char *rdstxt) {
	rdstxtOffset = 0xFFFF;
	DisplayText(rdstxt, 0);
}

void DisplayText(const char *rdstxt, short offset)
{
	if ((0)) {
		Serial.print("DisplayText[");
		Serial.print(offset);
		Serial.print(":");
		Serial.print(strlen(rdstxt));
		Serial.print("]");
		Serial.println(rdstxt);
	}
	const short txtleng = strlen(rdstxt);
	if (!txtleng) {
		//	dont clear display
		return;
	}

// 	if (rot_state == STATE_FREQ)
	{
		short ibuf;
		short irds = offset;
		for (ibuf=0; ibuf < TextBufferLength-1 && irds < txtleng; ++ibuf) {
			TextBuffer[ibuf] = rdstxt[irds++];
		}
		for (; ibuf < TextBufferLength-1; ++ibuf) {
			TextBuffer[ibuf] = ' ';
		}
		TextBuffer[TextBufferLength-1] = 0;
		//
		lcd.setCursor(0, 0);
		lcd.print(TextBuffer);
	}
} // DisplayText()



void DisplayTime(uint16_t minutes) {
	const uint16_t *blocks = radio.getRDSBlocks();
	snprintf(TextBuffer, sizeof(TextBuffer), "%04X:%04X; ", blocks[3], blocks[4]);
	Serial.print("raw time:");
	Serial.print(TextBuffer);

	int i = snprintf(TextBuffer, sizeof(TextBuffer), "TM:%02d:%02d", (int)minutes/60, (int)minutes%60);
	Serial.println(TextBuffer);
} // DisplayTime()

//	the end
