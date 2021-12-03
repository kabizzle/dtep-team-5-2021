/* s-Sense BME280 I2C / s-Sense BMP280 I2C sensor breakout example - v1.0/20190524. 
 * 
 * Compatible with:
 *    s-Sense BME280 I2C sensor breakout - temperature, humidity and pressure - [PN: SS-BME280#I2C, SKU: ITBP-6002], info https://itbrainpower.net/sensors/BME280-TEMPERATURE-HUMIDITY-PRESSURE-I2C-sensor-breakout 
 *    s-Sense BMP280 I2C sensor breakout - temperature and pressure - [PN: SS-BMP280#I2C, SKU: ITBP-6001], info https://itbrainpower.net/sensors/BMP280-TEMPERATURE-HUMIDITY-I2C-sensor-breakout
 *
 * This code shows how to use predefined recommended settings from Bosch for the BME280/BMP280 environmental sensor. Read temperature,  
 * humidity (unavailable for BMP280) and pressure (pulling at 1sec) - code based on BME280-2.3.0 library originally written by Tyler Glenn 
 * and forked by Alex Shavlovsky. Some part of code was written by Brian McNoldy. 
 * Amazing work folks! 
 * 
 * We've just select the relevant functions, add some variables, functions and fuctionalities.
 * 
 * 
 * Mandatory wiring:
 *    Common for 3.3V and 5V Arduino boards:
 *        sensor I2C SDA  <------> Arduino I2C SDA
 *        sensor I2C SCL  <------> Arduino I2C SCL
 *        sensor GND      <------> Arduino GND
 *    For Arduino 3.3V compatible:
 *        sensor Vin      <------> Arduino 3.3V
 *    For Arduino 5V compatible:
 *        sensor Vin      <------> Arduino 5V
 * 
 * Leave other sensor PADS not connected.
 * 
 * SPECIAL note for some ARDUINO boards:
 *        SDA (Serial Data)   ->  A4 on Uno/Pro-Mini, 20 on Mega2560/Due, 2 Leonardo/Pro-Micro
 *        SCK (Serial Clock)  ->  A5 on Uno/Pro-Mini, 21 on Mega2560/Due, 3 Leonardo/Pro-Micro
 * 
 * WIRING WARNING: wrong wiring may damage your Arduino board MCU or your sensor! Double check what you've done.
 * 
 * READ BME280 documentation! https://itbrainpower.net/sensors/BME280-TEMPERATURE-HUMIDITY-PRESSURE-I2C-sensor-breakout
 * READ BMP280 documentation! https://itbrainpower.net/sensors/BMP280-TEMPERATURE-PRESSURE-I2C-sensor-breakout
 * 
 * We ask you to use this SOFTWARE only in conjunction with s-Sense BME280 I2C or s-Sense BMP280 I2C sensor breakout usage. Modifications, derivates 
 * and redistribution of this SOFTWARE must include unmodified this notice. You can redistribute this SOFTWARE and/or modify it under the 
 * terms of this notice. 
 * 
 * This SOFTWARE is distributed is provide "AS IS" in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  
 * itbrainpower.net invests significant time and resources providing those how to and in design phase of our IoT products.
 * Support us by purchasing our environmental and air quality sensors from https://itbrainpower.net/order#s-Sense
 *
 *
 * Dragos Iosub, Bucharest 2019.
 * https://itbrainpower.net
 */

#define SERIAL_SPEED  19200

#include <sSense-BMx280I2C.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>


/* Recommended Modes -
   Based on Bosch BME280I2C environmental sensor data sheet.

Weather Monitoring :
   forced mode, 1 sample/minute
   pressure Ã—1, temperature Ã—1, humidity Ã—1, filter off
   Current Consumption =  0.16 Î¼A
   RMS Noise = 3.3 Pa/30 cm, 0.07 %RH
   Data Output Rate 1/60 Hz

Humidity Sensing :
   forced mode, 1 sample/second
   pressure Ã—0, temperature Ã—1, humidity Ã—1, filter off
   Current Consumption = 2.9 Î¼A
   RMS Noise = 0.07 %RH
   Data Output Rate =  1 Hz

Indoor Navigation :
   normal mode, standby time = 0.5ms
   pressure Ã—16, temperature Ã—2, humidity Ã—1, filter = x16
   Current Consumption = 633 Î¼A
   RMS Noise = 0.2 Pa/1.7 cm
   Data Output Rate = 25Hz
   Filter Bandwidth = 0.53 Hz
   Response Time (75%) = 0.9 s


Gaming :
   normal mode, standby time = 0.5ms
   pressure Ã—4, temperature Ã—1, humidity Ã—0, filter = x16
   Current Consumption = 581 Î¼A
   RMS Noise = 0.3 Pa/2.5 cm
   Data Output Rate = 83 Hz
   Filter Bandwidth = 1.75 Hz
   Response Time (75%) = 0.3 s

*/
LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x3F for a 16 chars and 2 line display

BMx280I2C::Settings settings(
   BME280::OSR_X1,
   BME280::OSR_X1,
   BME280::OSR_X1,
   BME280::Mode_Forced,
   BME280::StandbyTime_1000ms,
   BME280::Filter_Off,
   BME280::SpiEnable_False,
   0x76 // I2C address. I2C specific.
);

BMx280I2C ssenseBMx280(settings);

//////////////////////////////////////////////////////////////////
void setup()
{

  lcd.init();
  lcd.clear();         
  lcd.backlight();      // Make sure backlight is on
  
  delay(5000);
  DebugPort.begin(SERIAL_SPEED);

  while(!DebugPort) {} // Wait

  DebugPort.println("s-Sense BME/BMP280 I2C sensor.");

  Wire.begin();
  while(!ssenseBMx280.begin())
  {
    DebugPort.println("Could not find BME/BMP280 sensor!");
    delay(1000);
  }
  
  switch(ssenseBMx280.chipModel())
  {
     case BME280::ChipModel_BME280:
       DebugPort.println("Found BME280 sensor! Humidity available.");
       break;
     case BME280::ChipModel_BMP280:
       DebugPort.println("Found BMP280 sensor! No Humidity available.");
       break;
     default:
       DebugPort.println("Found UNKNOWN sensor! Error!");
  }

   // Change some settings before using.
   settings.tempOSR = BME280::OSR_X4;

   ssenseBMx280.setSettings(settings);
}

//////////////////////////////////////////////////////////////////
void loop()
{
   printBMx280Data(&DebugPort);
   delay(500);
}

//////////////////////////////////////////////////////////////////
void printBMx280Data
(
   Stream* client
)
{
   float temp(NAN), hum(NAN), pres(NAN);

   BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
   BME280::PresUnit presUnit(BME280::PresUnit_Pa);

   ssenseBMx280.read(pres, temp, hum, tempUnit, presUnit);

   client->print("Temp: ");
   client->print(temp);
   //client->print("Â°"+ String(tempUnit == BME280::TempUnit_Celsius ? 'C' :'F'));
   client->print(" "+ String(tempUnit == BME280::TempUnit_Celsius ? 'C' :'F'));
   client->print("\t\tHumidity: ");
   client->print(hum);
   client->print("% RH");
   client->print("\t\tPressure: ");
   client->print(pres);
   client->println(" Pa");

   lcd.setCursor(0,0);
   lcd.print("Room Temp");
   lcd.setCursor(10,0);
   lcd.print(temp);
   lcd.setCursor(0,1);
   lcd.print("Humidity");
   lcd.setCursor(10,1);
   lcd.print(hum);
  
   delay(3000);
}
