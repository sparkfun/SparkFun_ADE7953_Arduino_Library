/*
  Example 02 - Dual Channel Current

  Reads the RMS current from both Channel A and Channel B and prints them side by side.
  Channel A is the primary CT clamp input on the Qwiic connector. Channel B is available
  via the optional pinout headers on the board.

  begin() configures default gain on both channels for you, so this sketch just reads
  the current in amps from each channel.

  SparkFun Electronics
  Date: 2025
  SparkFun code, firmware, and software is released under the MIT License.
    Please see LICENSE.md for further details.

  Hardware Connections:
  IoT RedBoard --> ADE7953
  QWIIC --> QWIIC
  (Optional) Current source --> Channel B header pins

  Open the Serial Monitor at 115200 baud.

  Feel like supporting our work? Buy a board from SparkFun!
  https://www.sparkfun.com/sparkfun-qwiic-current-sensor.html
*/

#include <SparkFun_ADE7953.h>

SfeADE7953ArdI2C mySensor;

void setup()
{
    Serial.begin(115200);
    Serial.println("SparkFun ADE7953 Example 2 - Dual Channel Current");

    Wire.begin();

    while (mySensor.begin() == false)
    {
        Serial.println("ADE7953 not connected, check your wiring!");
        delay(1000);
    }

    Serial.println("ADE7953 connected!");

    // Tell the library which clamp is attached so the amps conversion is correct. This sets the
    // turns ratio and a suitable PGA gain for you. Use ADE7953_CLAMP_SCT013 for the SCT-013-000,
    // or mySensor.setCurrentClamp(yourRatio) to enter a custom turns ratio.
    mySensor.setCurrentClamp(ADE7953_CLAMP_ECS1030);

    Serial.println();
    Serial.println("Channel A (A)\tChannel B (A)");
    Serial.println("-------------\t-------------");
}

void loop()
{
    // Read the RMS current from both channels in amps.
    float ampsA = 0.0;
    float ampsB = 0.0;
    mySensor.getCurrentA(ampsA);
    mySensor.getCurrentB(ampsB);

    Serial.print(ampsA, 4);
    Serial.print("\t\t");
    Serial.println(ampsB, 4);

    delay(500);
}
