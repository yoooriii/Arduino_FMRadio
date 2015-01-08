///
/// \file RDSParser.h
/// \brief RDS Parser class definition.
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

#ifndef __RDSPARSER_H__
#define __RDSPARSER_H__

#include <arduino.h>

/// callback function for passing a ServicenName
extern "C" {
  typedef void(*receiveServicenNameFunction)(const char *name);
  typedef void(*receiveTextFunction)(const char *rdstxt);
  typedef void(*receiveTimeFunction)(uint16_t minutes);
}

/// Library for parsing RDS data values and extracting information.

class RDSParser2
{
public:
  RDSParser2();
  void reset();
  /// Pass all available RDS data through this function (4 words)
  void processData(const uint16_t *blocks);
  const char * getRDSText();

  void attachServicenNameCallback(receiveServicenNameFunction newFunction); ///< Register function for displaying a new Service Name.
  void attachTextCallback(receiveTextFunction newFunction); ///< Register the function for displaying a rds text.
  void attachTimeCallback(receiveTimeFunction newFunction); ///< Register function for displaying a new time

private:
	// ----- actual RDS values
	uint8_t rdsTP, rdsPTY;
	uint8_t _textAB, _last_textAB, _lastTextIDX;

	// Program Service Name
	char _PSName1[10]; // including trailing '\00' character.
	char _PSName2[10]; // including trailing '\00' character.
	char programServiceName[10];    // found station name or empty. Is max. 8 character long.
	uint16_t _lastRDSMinutes; ///< last RDS time send to callback.
	char _RDSText[64 + 2];

	//	Callbacks
	receiveServicenNameFunction _sendServiceName; ///< Registered ServiceName function.
	receiveTimeFunction _sendTime; ///< Registered Time function.
	receiveTextFunction _sendText;
}; //RDSParser


#endif //__RDSPARSER_H__
