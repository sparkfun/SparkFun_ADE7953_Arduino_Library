/*
  Example 01 - Basic Current Reading

  This example shows how to set up the ADE7953 current sensor with default
  settings and read the RMS current from Channel A (CT clamp input).

  The PGA gain is set to 4x, which is the correct setting for the 5.6 ohm
  shunt resistor on the SparkFun Qwiic Current Sensor board.

  SparkFun Electronics
  Date: 2025
  SparkFun code, firmware, and software is released under the MIT License.
    Please see LICENSE.md for further details.

  Hardware Connections:
  IoT RedBoard --> ADE7953
  QWIIC --> QWIIC

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
    Serial.println("ADE7953 Example 01 - Basic Current Reading");

    Wire.begin();

    // Initialize the ADE7953 at default I2C address (0x38).
    if (!mySensor.begin())
    {
        Serial.println("ADE7953 not found. Please check wiring. Freezing...");
        while (1)
            delay(1000);
    }

    Serial.println("ADE7953 connected!");

    // Set PGA gain to 4x for the 5.6 ohm shunt resistor on the board.
    if (!mySensor.setGainIA(ADE7953_PGA_GAIN_4))
    {
        Serial.println("Failed to set gain. Freezing...");
        while (1)
            delay(1000);
    }

    Serial.println("Gain set to 4x. Reading current...");
    Serial.println();
}

void loop()
{
    // Read the RMS current value from Channel A.
    uint32_t irms = mySensor.getIRmsA();

    Serial.print("IRMS Channel A (raw): ");
    Serial.println(irms);

    delay(500);
}
