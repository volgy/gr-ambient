// WeatherDuinoTest
//
// Copyright (C) 2015 Andras Nadas
// $Id: Weather_RF22.h,v 1.0 2015/01/21 21:02:32 nadand Exp $

#include <SPI.h>
#include <RHSoftwareSPI.h>
#include <Weather_RF22.h>
// include the library code:
#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(9, 8, 7, 6, 5, 4);

int linValue = 0;
int initError;
int rcvCnt = 0;

Weather_RF22::WeatherMesurement measurement;

// Singleton instance of the software SPI driver for Leonardo
RHSoftwareSPI spi;
// Singleton instance of the radio driver
Weather_RF22 rf22(SS, 2, spi);

void setup() {
  //Set up the serial for the measurement dump
  Serial.begin(9600); 
  // set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);
  // Print a Welcome message to the LCD.
  lcd.print("RFM22v");
  //Make sure Radio is up before we try to set it up
  delay(100);       
  
  // Initialise the Radio
  if (!rf22.init()){
    initError = rf22.lastError;
  }else{
    initError = 0; 
  }
}

void loop() {
  if(initError != 0){
    Serial.print("init failed ");
    Serial.println(initError);
  }else{
   lcd.setCursor(6, 0);  
   linValue = rf22.spiRead(RH_RF22_REG_01_VERSION_CODE);
   lcd.print(linValue);
    
    rf22.setModeRx();
    
    //Wait for incomming transmission
    if (rf22.waitAvailableTimeout(100))
    { 
      rcvCnt++;
      
      // Should have a message with measurements now   
      if (rf22.recvMeasurement(&measurement))
      {
        //Print it to the serial 
        Serial.print("channel:" );
        Serial.print(measurement.channel);
        Serial.print(" temp:" );
        Serial.print(measurement.temperature);
        Serial.print(" humidity:" );
        Serial.print(measurement.humidity);
        Serial.print(" battery warning:" );
        Serial.print(measurement.battery);
        Serial.print(" rssi:" );
        Serial.println(measurement.rssi);
        
        //Print it to the LCD
        lcd.setCursor(9, 0);
        lcd.print("Ch:");
        lcd.print(measurement.channel);
        lcd.print("  ");
        lcd.setCursor(0, 1);
        lcd.print("T:");
        lcd.print(measurement.temperature);
        lcd.print("F H:");
        lcd.print(measurement.humidity);
        lcd.print("%  ");
      }
      else
      {
        //Got a message with garbled bits: 
        //print the RSSI of the message for debugging
        Serial.print("recv failed with rssi:");
        Serial.println(rf22.lastRssi());
      }
    }
    /*
    //Debug code for checking radio reception on the LCD 
    lcd.setCursor(9, 0);
    lcd.print("rcv ");
    lcd.print(rcvCnt);
    lcd.setCursor(0, 1);
    lcd.print("rssi ");
    linValue = rf22.rssiRead();
    lcd.print(linValue);
    lcd.print(" c ");
    linValue = rf22.rxGood();
    lcd.print(linValue);
    lcd.print("|");
    linValue = rf22.rxBad();
    lcd.print(linValue);
    lcd.print("  ");*/    
  }     
  
  // Just to be safe and not drive the loop at 100%
  delay(10);
  
}

