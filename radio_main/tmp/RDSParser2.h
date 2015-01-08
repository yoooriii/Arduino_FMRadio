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

#ifndef __RDSPARSER2_H__
#define __RDSPARSER2_H__

#include <arduino.h>

/// callback function for passing a ServicenName
extern "C" {
  typedef void(*receiveServicenNameFunction)(char *name);
  typedef void(*receiveTextFunction)(char *name);
  typedef void(*receiveTimeFunction)(uint8_t hour, uint8_t minute);
}

/// Library for parsing RDS data values and extracting information.

class RDSParser2
{
public:
	RDSParser2();
	void init();
	void attachServicenNameCallback(receiveServicenNameFunction newFunction); ///< Register function for displaying a new Service Name.
	void attachTimeCallback(receiveTimeFunction newFunction); ///< Register function for displaying a new time

}; //RDSParser2


#endif //__RDSPARSER2_H__
