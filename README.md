gr-ambient
==========

GNU Radio blocks for receiving reports from Ambient Weather wireless sensors. 

Tested w/ Ambient Weather F007TH Wireless Thermo-Hygrometer

Arduino Implementation
--------------
The implementation was done using the HopeRF RFM22B board configured to be an OOK receiver and Manchester decoder. 

The WeatherRF library is an extension to the excellent RadioHead (http://www.airspayce.com/mikem/arduino/RadioHead/index.html) library.

The WeatherDuinoDev is a small Arduino test app to write the incoming temperature and humidity measures onto the Serial port and to an LCD display. 
