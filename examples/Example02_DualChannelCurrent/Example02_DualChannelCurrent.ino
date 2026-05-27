/*
  Example 02 - Dual Channel Current

  This example reads the RMS current from both Channel A and Channel B
  simultaneously and prints them side by side. Channel A is the primary
  CT clamp input on the Qwiic connector. Channel B is available via the
  optional pinout headers on the board.

  SparkFun Electronics
  Date: 2025
  SparkFun code, firmware, and software is released under the MIT License.
    Please see LICENSE.md for further details.

  Hardware Connections:
  IoT RedBoard --> ADE7953
  QWIIC --> QWIIC
  (Optional) Current source --> Channel B header pins

  Serial.print it out at 115200 baud to serial monitor.

  Feel like supporting our work? Buy a board from SparkFun!
  https://www.sparkfun.com/sparkfun-qwiic-current-sensor.html
*/

#include <SparkFun_ADE7953.h>

SfeADE7953ArdI2C mySensor;

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("ADE7953 Example 02 - Dual Channel Current");

    Wire.begin();

    if (!mySensor.begin())
    {
        Serial.println("ADE7953 not found. Please check wiring. Freezing...");
        while (1)
            delay(1000);
    }

    Serial.println("ADE7953 connected!");

    // Set PGA gain to 4x on both channels for the 5.6 ohm shunt resistor.
    mySensor.setGainIA(ADE7953_PGA_GAIN_4);
    mySensor.setGainIB(ADE7953_PGA_GAIN_4);

    Serial.println("Both channels configured at 4x gain.");
    Serial.println();

    // Print column headers.
    Serial.println("Channel A (raw)\tChannel B (raw)");
    Serial.println("---------------\t---------------");
}

void loop()
{
    // Read the RMS current from both channels.
    uint32_t irmsA = mySensor.getIRmsA();
    uint32_t irmsB = mySensor.getIRmsB();

    Serial.print(irmsA);
    Serial.print("\t\t");
    Serial.println(irmsB);

    delay(500);
}
