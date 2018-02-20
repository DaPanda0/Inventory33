#include <Arduino.h>
#include <SPI.h>
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"

#include "BluefruitConfig.h"

#if SOFTWARE_SERIAL_AVAILABLE
#include <SoftwareSerial.h>
#endif
#define FACTORYRESET_ENABLE         0
#define MINIMUM_FIRMWARE_VERSION    "0.6.6"
#define MODE_LED_BEHAVIOUR          "MODE"
/*=========================================================================*/


Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

/* ...software SPI, using SCK/MOSI/MISO user-defined SPI pins and then user selected CS/IRQ/RST */
//Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_SCK, BLUEFRUIT_SPI_MISO,
//                             BLUEFRUIT_SPI_MOSI, BLUEFRUIT_SPI_CS,
//                             BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);


// A small helper
void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}

/**************************************************************************/
/*!
    @brief  Sets up the HW an the BLE module (this function is called
            automatically on startup)
*/
/**************************************************************************/

# include "HX711.h"
HX711 scale(A2, A3);
float weight;


bool combined () {
  if (weight < -1 || weight > 1 ) {
    return true;
  }
  else {
    return false;
  }
}

void setup () {
  while (!Serial);  // required for Flora & Micro
  delay(500);

  Serial.begin(115200);
  Serial.println(F("Adafruit Bluefruit Command <-> Data Mode Example"));
  Serial.println(F("------------------------------------------------"));

  /* Initialise the module */
  Serial.print(F("Initialising the Bluefruit LE module: "));

  if ( !ble.begin(VERBOSE_MODE) )
  {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  Serial.println( F("OK!") );

  if ( FACTORYRESET_ENABLE )
  {
    /* Perform a factory reset to make sure everything is in a known state */
    Serial.println(F("Performing a factory reset: "));
    if ( ! ble.factoryReset() ) {
      error(F("Couldn't factory reset"));
    }
  }

  /* Disable command echo from Bluefruit */
  ble.echo(false);

  Serial.println("Requesting Bluefruit info:");
  /* Print Bluefruit information */
  ble.info();



  ble.verbose(false);  // debug info is a little annoying after this point!

  /* Wait for connection */
  while (! ble.isConnected()) {
    delay(500);
  }

  Serial.println(F("******************************"));

  // LED Activity command is only supported from 0.6.6
  if ( ble.isVersionAtLeast(MINIMUM_FIRMWARE_VERSION) )
  {
    // Change Mode LED Activity
    Serial.println(F("Change LED activity to " MODE_LED_BEHAVIOUR));
    ble.sendCommandCheckOK("AT+HWModeLED=" MODE_LED_BEHAVIOUR);
  }

  //Give module a new name
  ble.println("AT+GAPDEVNAME=Fairmaiden"); // named LONE

  // Check response status
  ble.waitForOK();
  // Set module to DATA mode
  Serial.println( F("Switching to DATA mode!") );
  ble.setMode(BLUEFRUIT_MODE_DATA);

  Serial.println(F("******************************"));


  /**************************************************************************/
  /*!
      @brief  Constantly poll for new command or response data
  */
  /**************************************************************************/

  scale.tare(); // reset the scale to 0
  scale.set_scale(2280.f); // this value is obtained by calibrating the scale with known weights
}

void loop () {
  // Check for user input
  char n, inputs[BUFSIZE + 1];

  if (Serial.available())
  {
    n = Serial.readBytes(inputs, BUFSIZE);
    inputs[n] = 0;
    // Send characters to Bluefruit
    Serial.print("Sending: ");
    Serial.println(inputs);

    // Send input data to host via Bluefruit
    ble.print(inputs);
  }
  if (ble.available()) {
    Serial.print("* "); Serial.print(ble.available()); Serial.println(F(" bytes available from BTLE"));
  }
  // Echo received data
  while ( ble.available() )
  {
    int c = ble.read();
    Serial.print((char)c);
  }
  delay(1000);


  weight = scale.get_units(); // read the weight and store the value in the 'weight' variable
  //combined ();
  
  // Serial.println(weight);
  /*Serial.print ("weight in grams: ");
    Serial.println(weight ); // print the weight over serial connection*/
  
  ble.print(weight);

  if (combined() == true) {
    ble.print("Inventory is low");
  }


}


/*#include "HX711.h"



  // HX711.DOUT  - pin #A1

  // HX711.PD_SCK - pin #A0



  HX711 scale(A1, A0);



  float read_ADC;

  float read_load;

  float read_average;



  void setup() {

  Serial.begin(38400);

  scale.set_scale(400.f); // this value is obtained by calibrating the scale with known weights; see the README for details

  scale.tare();               // reset the scale to 0



  Serial.print("read: \t\t");

  Serial.println(scale.read());  // print a raw reading from the ADC



  Serial.print("read average: \t\t");

  Serial.println(scale.read_average(20));  // print the average of 20 readings from the ADC



  Serial.print("get units: \t\t");

  Serial.println(scale.get_units(5), 1);        // print the average of 5 readings from the ADC minus tare weight, divided

  // by the SCALE parameter set with set_scale



  Serial.println("Readings:");

  }



  void loop() {

  read_ADC   =   scale.read();

  Serial.print("read: \t");

  Serial.println(read_ADC);                 // print a raw reading from the ADC



  read_load   =    scale.get_units();



  Serial.print("One reading:\t");

  Serial.print(read_load, 1);

  Serial.println("  Gram");



  read_average   =    scale.get_units(10);

  Serial.print("Average:\t");

  Serial.print(read_average, 1); // print the average of 10 readings from the ADC minus tare weight, divided

  // by the SCALE parameter set with set_scale

  Serial.println("  Gram");



  delay(1000);
  }
*/
