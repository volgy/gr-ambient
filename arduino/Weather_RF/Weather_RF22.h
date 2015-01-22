// Weather_RF22.h
//
// Copyright (C) 2015 Andras Nadas
// $Id: Weather_RF22.h,v 1.0 2015/01/18 10:58:47 nadand Exp $

#ifndef WEATHER_RF22_h
#define WEATHER_RF22_h

#include <RH_RF22.h>

class Weather_RF22 : public RH_RF22
{
public:

	typedef enum
    {
		OOK_M_Rb1Bw200=0,        ///< OOK, Manchester, Rb = 1kbs,   Rx Bandwidth = 200kHz
		OOK_M_Rb1Bw100,          ///< OOK, Manchester, Rb = 1kbs,   Rx Bandwidth = 100kHz
		OOK_M_Rb1Bw400,          ///< OOK, Manchester, Rb = 1kbs,   Rx Bandwidth = 400kHz
		OOK_M_Rb1024Bw200,    	 ///< OOK, Manchester, Rb = 1.024kbs,   Rx Bandwidth = 200kHz
		OOK_Rb1Bw200,			 ///< OOK, without Manchester, Rb = 1kbs,   Rx Bandwidth = 200kHz
		OOK_Rb2Bw200,			 ///< OOK, without Manchester, Rb = 2kbs,   Rx Bandwidth = 200kHz
		OOK_MD_Rb1Bw200,		 ///< OOK, Manchester Data, Rb = 1kbs,   Rx Bandwidth = 200kHz

    } WModemConfigChoice;

    typedef struct
    {
    	uint8_t channel;	///channel of the sensor 1-8
    	float temperature;	///Temperature measured in Farenheit 
    	float humidity;		///Humidity measured in %
    	bool battery;		///Battery low warning
    	int rawTemp;		///raw data of temperature
    	int rawHumidity;	///raw humidity data
    	int8_t rssi;		///radio signal strength indicator for the message
    } WeatherMesurement;

    /// Constructor. See RH_RF22.h for parameter description.
	Weather_RF22(uint8_t slaveSelectPin = SS, uint8_t interruptPin = 2, RHGenericSPI& spi = hardware_spi);

	/// Select one of the predefined modem configurations. If you need a modem configuration not provided 
    /// here, use setModemRegisters() with your own ModemConfig.
    /// \param[in] index The configuration choice.
    /// \return true if index is a valid choice.
    bool        setWModemConfig(WModemConfigChoice index);

    // Initialises this instance and the radio module connected to it.
    /// The following steps are taken:
    /// - Initialise the RH_RF22 library
    /// - Sets the Modem parameters
    /// - Sets the message format
    /// \return  true if everything was successful
    bool 		init();

    /// Turns the receiver on if it not already on.
    /// If there is a valid radio message available, interpret measurement and copy it to buf and return true
    /// else return false.
    /// You should be sure to call this function frequently enough to not miss any messages
    /// It is recommended that you call it in your main loop.
    /// \param[in] buf Location to copy the received measurements
    /// \return true if a valid measurement was copied to buf
    bool        recvMeasurement(WeatherMesurement* buf);

    /// Returns the count of the number of damaged received packets (ie packets with bit errors in the data ,etc)
    /// which were rejected and not delivered to the application.
    /// \return The number of damaged packets received.
    uint16_t       rxDamaged();

protected:

	uint8_t _lastError;
	/// Count of the number of damage messages (eg bad checksum etc) received
    volatile uint16_t   _rxDamaged;

	uint8_t buf[RH_RF22_MAX_MESSAGE_LEN];
    

};
#endif