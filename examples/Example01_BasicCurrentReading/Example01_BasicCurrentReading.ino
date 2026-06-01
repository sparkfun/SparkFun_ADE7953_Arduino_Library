/*
  Example 01 - Basic Current Reading

  The simplest way to read current from the SparkFun Qwiic Current Sensor (ADE7953).
  begin() configures sensible defaults (PGA gain and high-pass filter) for you, so all
  this sketch has to do is ask for the current in amps.

  SparkFun Electronics
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
}

void loop()
{
    // Read the current on Channel A in amps.
    float amps = 0.0;
    mySensor.getCurrentA(amps);

    // Print the current.
    Serial.print("Current (A): ");
    Serial.println(amps, 4);

    // Slow things down so we are not spamming the output.
    delay(1000);
}
