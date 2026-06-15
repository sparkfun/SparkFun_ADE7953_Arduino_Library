/*
  Example 01 - Basic Current Reading

  The simplest way to read current from the SparkFun Qwiic Current Sensor (ADE7953).
  begin() configures 16x PGA gain and enables the high-pass filter. After connecting,
  this sketch runs a quick auto-calibration (clamp open, no load) to zero the noise
  floor before reporting current.

  SparkFun Electronic
  Date: 2025
  SparkFun code, firmware, and software is released under the MIT License.
    Please see LICENSE.md for further details.

  Hardware Connections:
  IoT RedBoard --> ADE7953
  QWIIC --> QWIIC

  Open the Serial Monitor at 115200 baud.

  Feel like supporting our work? Buy a board from SparkFun!
  https://www.sparkfun.com/sparkfun-qwiic-current-sensor.html
*/

#include <SparkFun_ADE7953.h>

SfeADE7953ArdI2C mySensor;

void setup()
{
    // Start serial right away so we can report what is happening.
    Serial.begin(115200);
    Serial.println("SparkFun ADE7953 Example 1 - Basic Current Reading");

    // Start I2C communication.
    Wire.begin();

    // Attempt to connect to the sensor. Keep trying so the message is not missed if the
    // Serial Monitor is opened late.
    while (mySensor.begin() == false)
    {
        Serial.println("ADE7953 not connected, check your wiring!");
        delay(1000);
    }

    Serial.println("ADE7953 connected!");

    // Zero the noise floor. Make sure the CT clamp is open (no current flowing) before
    // this runs. The library subtracts this baseline in the quadrature domain so the
    // reading returns 0 A when there is no load.
    Serial.println("Calibrating baseline — keep CT clamp open with no current flowing...");
    mySensor.autoCalibrateA(50);
    Serial.println("Calibration done. Reading current...");
}

void loop()
{
    // Read the current on Channel A in amps.
    float amps = 0.0;
    mySensor.getCurrentA(amps);

    // Print the current.
    //Serial.print("Current: ");
    Serial.println(amps, 4);

    // Slow things down so we are not spamming the output.
    delay(250);
}
