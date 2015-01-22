// Weather_RF22.cpp
//
// Copyright (C) 2015 Andras Nadas
// $Id: Weather_RF22.cpp,v 1.0 2015/01/18 11:09:32 nadand Exp $

#include <Weather_RF22.h>

PROGMEM static const RH_RF22::ModemConfig WEATHER_MODEM_CONFIG_TABLE[] =
{
	// All the following enable FIFO with reg 71
    //  1c,   1f,   20,   21,   22,   23,   24,   25,   2c,   2d,   2e,   58,   69,   6e,   6f,   70,   71,   72
	// OOK, Manchester, Max Rb err <1%, Xtal Tol 20ppm
    { 0xc1, 0x03, 0x77, 0x20, 0x2b, 0xb1, 0x00, 0x59, 0x1a, 0x71, 0x2a, 0x80, 0x60, 0x08, 0x31, 0x2e, 0x21, 0x50 }, // 1.0, 200
    { 0x45, 0x03, 0x7d, 0x00, 0x83, 0x12, 0x01, 0x08, 0x1a, 0x71, 0x28, 0x80, 0x60, 0x08, 0x31, 0x2e, 0x21, 0x50 }, // 1.0, 100
    { 0xca, 0x03, 0x77, 0x20, 0x2b, 0xb1, 0x00, 0x59, 0x1a, 0x71, 0x2a, 0x80, 0x60, 0x08, 0x31, 0x2e, 0x21, 0x50 }, // 1.0, 400
    { 0xc1, 0x03, 0x6e, 0x20, 0x2c, 0xbd, 0x00, 0x5c, 0x1a, 0x62, 0x2a, 0x80, 0x60, 0x08, 0x63, 0x26, 0x21, 0x50 }, // 1.024, 200
    
    // OOK, No Manchester, Max Rb err <1%, Xtal Tol 20ppm
    { 0xd1, 0x03, 0x77, 0x20, 0x57, 0x62, 0x00, 0x59, 0x2c, 0xe2, 0x2a, 0x80, 0x60, 0x08, 0x31, 0x2c, 0x21, 0x50 }, // 1.0, 200
    { 0xc1, 0x03, 0x77, 0x20, 0x57, 0x62, 0x00, 0x59, 0x2a, 0x71, 0x2a, 0x80, 0x60, 0x10, 0x62, 0x2c, 0x21, 0x50 }, // 2.0, 200
    // OOK, Manchester Data, Max Rb err <1%, Xtal Tol 20ppm 
    { 0xd1, 0x03, 0x77, 0x20, 0x57, 0x62, 0x00, 0x59, 0x1c, 0xe2, 0x2a, 0x80, 0x60, 0x08, 0x31, 0x2c, 0x21, 0x50 }, // 1.0, 200
};


Weather_RF22::Weather_RF22(uint8_t slaveSelectPin, uint8_t interruptPin, RHGenericSPI& spi)
    :
    RH_RF22(slaveSelectPin, interruptPin, spi)
{
}

bool Weather_RF22::init()
{
	if (!RH_RF22::init()){
        lastError = 1;
        return false;
    }

    //ISM Frequency of the Weather sensors
    setFrequency(434.0, 0.05);
    //Radio Modem (Modulation and Encoding) configuration
    setWModemConfig(OOK_M_Rb1024Bw200);

    //Set preamble for 2 bytes; 8 x 1->0 transition from the 11 -3 will be in the sync wors
   	setPreambleLength(2);
    //Set the Sync word from the manchester encoded preamble + sync bits (11101010)
    //uint8_t syncwords[] = { 0xEA };
    uint8_t syncwords[] = { 0x51 };
    //uint8_t syncwords[] = { 0x45 };
    //uint8_t syncwords[] = { 0xa9, 0x99 };
    setSyncWords(syncwords, sizeof(syncwords));
    
    //uint8_t det = rf22.spiRead(RH_RF22_REG_35_PREAMBLE_DETECTION_CONTROL1);
    
    //Set: Preamble detectin to 4 nibble(4bits) and default rssi offset (b010)
    spiWrite(RH_RF22_REG_35_PREAMBLE_DETECTION_CONTROL1, 0x22); //4 nibble
    //spiWrite(RH_RF22_REG_35_PREAMBLE_DETECTION_CONTROL1, 0x1a); //3 nibble
    //spiWrite(RH_RF22_REG_35_PREAMBLE_DETECTION_CONTROL1, 0x12); //2 nibble
    //Disable: Broadcast Address Chceck, RX Header Bytes Check
    spiWrite(RH_RF22_REG_32_HEADER_CONTROL1, 0x00);
    //Header Lenght: 0; Sync word lenght: 1; Fixed Packet length
    spiWrite(RH_RF22_REG_33_HEADER_CONTROL2, 0x08);
    //Enable: RX Packet Handling, TX Packet Handling; Disabled: CRC generation;
    spiWrite(RH_RF22_REG_30_DATA_ACCESS_CONTROL, RH_RF22_ENPACRX | RH_RF22_ENPACTX );
    //Set Packet lenght to 25 (~ 195/8)
    spiWrite(RH_RF22_REG_3E_PACKET_LENGTH, 22);

    //spiWrite(RH_RF22_REG_70_MODULATION_CONTROL1, 0x22);
    //spiWrite(RH_RF22_REG_0D_GPIO_CONFIGURATION2,0x0c);
    //spiWrite(RH_RF22_REG_50_ANALOG_TEST_BUS_SELECT,0x01);
}

// Set one of the canned FSK Modem configs
// Returns true if its a valid choice
bool Weather_RF22::setWModemConfig(WModemConfigChoice index)
{
    if (index > (signed int)(sizeof(WEATHER_MODEM_CONFIG_TABLE) / sizeof(ModemConfig)))
        return false;

    RH_RF22::ModemConfig cfg;
    memcpy_P(&cfg, &WEATHER_MODEM_CONFIG_TABLE[index], sizeof(RH_RF22::ModemConfig));
    setModemRegisters(&cfg);

    return true;
}

bool Weather_RF22::recvMeasurement(WeatherMesurement* wm_buf)
{
	uint8_t len = sizeof(buf);
    if (recv(buf, &len)){
#if 0  	
		//Debug serial print of the whole message
    	Serial.print("got message! len: ");
        Serial.print(len);
        Serial.print(" [");
        int i;
        for (i = 0; i < len; i = i + 1) {
        	for( int j = 0; j< 8; j++){
        		if(buf[i] & (0x01) << 7-j){
					Serial.print("1");
        		}else{
        			Serial.print("0");
        		}
        	}
        }
        Serial.println("]");
#endif
		//Consistency checking
		// Adding all 4 bytes of measurements in all three repeats into three 32 bit varibles for consistency checking
		uint32_t m1 = (((((((uint32_t)buf[1]) << 8) | buf[2]) << 8) | buf[3]) << 8 ) | buf[4];
		uint32_t m2 = ((((((((((uint32_t)buf[9]) << 8) | buf[10]) << 8) | buf[11])) << 8 ) | buf[12]) << 1) | buf[13] >>7 ;
		uint32_t m3 = ((((((((((uint32_t)buf[17]) << 8) | buf[18]) << 8) | buf[19])) << 8 ) | buf[20]) << 2) | buf[21] >>6 ;
		if( m1 != m2 || m1 != m3 || m2 != m3) {
			//inconsistent repeats inside the package means bit errors; don't use the message.
			_rxDamaged++;
			return false;  
		}

		//Assemble measurement from packet
    	wm_buf->channel = 1 + ((buf[1] & 0x1c) >> 2); 
        wm_buf->rawTemp =  (buf[1]&0x01) * 2048 + buf[2] * 8 + (buf[3]>>5) ;
        wm_buf->rawHumidity = (buf[3]&0x1f) * 8 + (buf[4]>>5);
        wm_buf->temperature = -40.0 + ((float)wm_buf-> rawTemp )/20.0;
        wm_buf->humidity = ((float)wm_buf->rawHumidity)/2.0;
        wm_buf->battery = (bool) (buf[1] & 0x20);
        wm_buf->rssi = lastRssi();
    }else{
    	//reception failure
    	return false;
    }

	return true;
}

uint16_t Weather_RF22::rxDamaged(){
	return _rxDamaged;
}

