///
/// \file RDSParser.cpp
/// \brief RDS Parser class implementation.
///
/// \author Matthias Hertel, http://www.mathertel.de
/// \copyright Copyright (c) 2014 by Matthias Hertel.\n
/// This work is licensed under a BSD style license.\n
/// See http://www.mathertel.de/License.aspx
///
/// \details
///
/// More documentation and source code is available at http://www.mathertel.de/Arduino
///
/// History:
/// --------
/// * 01.09.2014 created and RDS sender name working.
/// * 01.11.2014 RDS time added.

#include "projectprefix.h"
#include "RDSParser.h"


#define DEBUG_FUNC0(fn)          { Serial.print(fn); Serial.println("()"); }

RDSParser2::RDSParser2() {
	memset(this, 0, sizeof(RDSParser2));
	reset();
} // RDSParser2()


void RDSParser2::reset() {
	LOGTHIS;
	strcpy(_PSName1, "--------");
	strcpy(_PSName2, _PSName1);
	strcpy(programServiceName, "          ");
	memset(_RDSText, 0, sizeof(_RDSText));
	_lastTextIDX = 0;
	_lastRDSMinutes = 0;
}


void RDSParser2::attachServicenNameCallback(receiveServicenNameFunction newFunction) {
	LOGTHIS;
	_sendServiceName = newFunction;
}

void RDSParser2::attachTextCallback(receiveTextFunction newFunction) {
	LOGTHIS;
	_sendText = newFunction;
}
//
void RDSParser2::attachTimeCallback(receiveTimeFunction newFunction) {
	LOGTHIS;
	_sendTime = newFunction;
}

const char * RDSParser2::getRDSText() {
	return _RDSText;
}

uint16_t decodeTimeBlocks(uint16_t block2, uint16_t block3, uint16_t block4) {

		unsigned char utcMinutesPrevious = 0;//tmp val, no need?
		unsigned int  modifiedJulianDay = 0;
		signed   char localHours = 0;
		unsigned char utcHours = 0;
		signed   char localMinutes = 0;
		unsigned char utcMinutes = 0;
		unsigned int  localTimeOffset = 0;
		unsigned char localSign = 0;
		unsigned int  utcYear = 0;
		unsigned char utcMonth = 0;
		unsigned char utcDay = 0;

        //bits 0-5 are in block4 as bits 6-11
        utcMinutes = (block4  / 64) & 0x3F;

        if (utcMinutesPrevious != utcMinutes) {

          utcMinutesPrevious = utcMinutes;

          //bits 0-14 are in block3 as bits 1-15
          //bits 15-16 are in block2 as bits 0-1
          modifiedJulianDay = (block3 / 2) + (block2 & 0x03) * 32768;

          //bits 0-3 are in block4 as bits 12-15
          //bit 4 is in block3 as bit 1
          utcHours = (block4 / 4096) + (block3 & 0x01) * 16;

          //local time offset are bits 0-4 in block 4
          localTimeOffset = block4 & 0x1F;
          //sign is in bit 5 of block4, 0=+ 1=-
          if (block4 & 0x20)
            localSign = '-';
          else
            localSign = '+';

          //multiply by 30 so that we have offset in minutes (offset is in multiples of .5 hours)
          localTimeOffset *= 30;

          printf_P(PSTR("CT: 0x%01X%04X%04X, "), (block2 & 0x03), block3, block4);

          //Modified Julian date to year-month-day conversion
          utcYear = floor((modifiedJulianDay - 15078.2) / 365.25);
          utcMonth = floor((modifiedJulianDay - 14956.1 - floor(utcYear * 365.25)) / 30.6001);
          utcDay = modifiedJulianDay - 14956 - floor(utcYear * 365.25) - floor(utcMonth * 30.6001);

		unsigned char i;
          if (utcMonth == 14 || utcMonth == 15)
            i = 1;
          else
            i = 0;

          utcYear = utcYear + i + 1900;
          utcMonth = utcMonth - 1 - (i * 12);

          printf_P(PSTR("UTC %04u-%02u-%02u (MJD %u) %02u:%02u:00 %c%02u:%02u, "),
                   utcYear, utcMonth, utcDay,
                   modifiedJulianDay,
                   utcHours, utcMinutes, localSign,
                   localTimeOffset / 60, localTimeOffset % 60);

          //TODO: half hour timezones and negative timezones not tested because lack of station transmitting it.
          //lets calulate local time
          if (localSign == '-') {
            localHours = utcHours - (localTimeOffset / 60);
            localMinutes = utcMinutes - (localTimeOffset % 60);
          } else {
            localHours = utcHours + (localTimeOffset / 60);
            localMinutes = utcMinutes + (localTimeOffset % 60);
          }

          if (localMinutes < 0) {
            localMinutes += 60;
            localHours--;
          }

          if (localMinutes > 59) {
            localMinutes -= 60;
            localHours++;
          }

          if (localHours < 0)
            localHours += 24;

          if (localHours > 23)
            localHours -= 24;

          printf_P(PSTR("TIME %02u:%02u:00\r\n"), localHours, localMinutes);
        }

	return 60*localHours+localMinutes;
}

void RDSParser2::processData(const uint16_t *blocks)
{
	uint8_t  idx; // index of rdsText

	// analyzing Block 2
	const uint8_t rdsGroupType = 0x0A | ((blocks[2] & 0xF000) >> 8) | ((blocks[2] & 0x0800) >> 11);
	rdsTP = (blocks[2] & 0x0400);
	rdsPTY = (blocks[2] & 0x0400);

	switch (rdsGroupType) {
		case 0x0A:
		case 0x0B: {
			char c1, c2;
			// The data received is part of the Service Station Name
			idx = 2 * (blocks[2] & 0x0003);

			// new data is 2 chars from block 4
			c1 = blocks[4] >> 8;
			c2 = blocks[4] & 0x00FF;

			// check that the data was received successfully twice
			// before publishing the station name

			if ((_PSName1[idx] == c1) && (_PSName1[idx + 1] == c2)) {
				// retrieved the text a second time: store to _PSName2
				_PSName2[idx] = c1;
				_PSName2[idx + 1] = c2;
				_PSName2[8] = '\0';

				if ((idx == 6) && strcmp(_PSName1, _PSName2) == 0) {
					if (strcmp(_PSName2, programServiceName) != 0) {
						// publish station name
						strcpy(programServiceName, _PSName2);
						if (_sendServiceName) {
							_sendServiceName(programServiceName);
						}
					}
				}
			}

			if ((_PSName1[idx] != c1) || (_PSName1[idx + 1] != c2)) {
				_PSName1[idx] = c1;
				_PSName1[idx + 1] = c2;
				_PSName1[8] = '\0';
				// Serial.println(_PSName1);
			} // if
		} break;

	case 0x2A: {
		// The data received is part of the RDS Text.
		_textAB = (blocks[2] & 0x0010);
		idx = 4 * (blocks[2] & 0x000F);

		if (idx < _lastTextIDX) {
		  // the existing text might be complete because the index is starting at the beginning again.
		  // now send it to the possible listener.
			if (_sendText) {
				_sendText(_RDSText);
			}
		}
		_lastTextIDX = idx;

		if (_textAB != _last_textAB) {
			// when this bit is toggled the whole buffer should be cleared.
			_last_textAB = _textAB;
			bzero(_RDSText, sizeof(_RDSText));
			Serial.println("T>CLEAR");
		} // if

		// new data is 2 chars from block 3
		_RDSText[idx++] = (blocks[3] >> 8);
		_RDSText[idx++] = (blocks[3] & 0x00FF);

		// new data is 2 chars from block 4
		_RDSText[idx++] = (blocks[4] >> 8);
		_RDSText[idx++] = (blocks[4] & 0x00FF);

		char *p = strchr(_RDSText, '\0x0D');
		if (p) {
			*p = ']';
		}

	} break;

	case 0x4A: {
	//TODO: I hope this works
		const uint16_t tminutes = decodeTimeBlocks(blocks[2], blocks[3], blocks[4]);
		if (_lastRDSMinutes != tminutes) {
			_lastRDSMinutes = tminutes;
			if (_sendTime) {
				_sendTime(_lastRDSMinutes);
			}
		}
		break;


		//	Clock time and date
		//TODO: there is smh wrong with it
		//	RDS time in minutes
		uint16_t mins = (blocks[4] >> 6) & 0x3F; // 6 bits
		mins += 60 * (((blocks[3] & 0x0001) << 4) | ((blocks[4] >> 12) & 0x0F));

		//	RDS time offset and sign
		//	adjust offset
		int16_t offset = 30 * (blocks[4] & 0x1F);
		if (blocks[4] & 0x20) {
			offset = -offset;
		}
		mins += offset;

		if ((_sendTime) && (mins != _lastRDSMinutes)) {
			_lastRDSMinutes = mins;
			_sendTime(_lastRDSMinutes);
		} // if
	} break;

	case 0x6A:
    // IH
		break;

	case 0x8A:
    // TMC
		break;

	case 0xAA:
    // TMC
		break;

	case 0xCA:
    // TMC
		break;

	case 0xEA:
    // IH
		break;

	default:
    // Serial.print("RDS_GRP:"); Serial.println(rdsGroupType, HEX);
		break;
	}
}
