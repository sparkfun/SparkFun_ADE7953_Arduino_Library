/*
  Example 03 - Gain Configuration

  Demonstrates how to configure the PGA (Programmable Gain Amplifier) and the digital
  (fine) gain on Channel A.

  The PGA gain controls the amplifier sensitivity. 16x is the default for CT clamp use.
  Available PGA gain settings:
    ADE7953_PGA_GAIN_1   (1x)
    ADE7953_PGA_GAIN_2   (2x)
    ADE7953_PGA_GAIN_4   (4x)
    ADE7953_PGA_GAIN_8   (8x)
    ADE7953_PGA_GAIN_16  (16x) <-- default
    ADE7953_PGA_GAIN_22  (22x, current channels only)

  The digital gain provides fine-tuning on top of the PGA. This example sets it with a
  simple floating-point multiplier (1.0 = unity) instead of a raw register value.

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

// Helper to print the PGA gain as a human-readable string.
void printGain(sfe_ade7953_pga_gain_t gain)
{
    switch (gain)
    {
    case ADE7953_PGA_GAIN_1:
        Serial.print("1x");
        break;
    case ADE7953_PGA_GAIN_2:
        Serial.print("2x");
        break;
    case ADE7953_PGA_GAIN_4:
        Serial.print("4x");
        break;
    case ADE7953_PGA_GAIN_8:
        Serial.print("8x");
        break;
    case ADE7953_PGA_GAIN_16:
        Serial.print("16x");
        break;
    case ADE7953_PGA_GAIN_22:
        Serial.print("22x");
        break;
    default:
        Serial.print("Unknown");
        break;
    }
}

void setup()
{
    Serial.begin(115200);
    Serial.println("SparkFun ADE7953 Example 3 - Gain Configuration");

    Wire.begin();

    while (mySensor.begin() == false)
    {
        Serial.println("ADE7953 not connected, check your wiring!");
        delay(1000);
    }

    Serial.println("ADE7953 connected!");
    Serial.println();

    // Read the current PGA gain on Channel A (set to 16x by begin()).
    sfe_ade7953_pga_gain_t currentGain;
    mySensor.getGainIA(currentGain);
    Serial.print("PGA gain (Channel A): ");
    printGain(currentGain);
    Serial.println();

    // Change the PGA gain to 8x.
    Serial.println("Setting PGA gain to 8x...");
    mySensor.setGainIA(ADE7953_PGA_GAIN_8);

    mySensor.getGainIA(currentGain);
    Serial.print("PGA gain is now: ");
    printGain(currentGain);
    Serial.println();
    Serial.println();

    // Read the digital gain as a multiplier (1.0 = unity).
    float digitalGain = 0.0;
    mySensor.getDigitalGainIA(digitalGain);
    Serial.print("Digital gain multiplier: ");
    Serial.println(digitalGain, 3);

    // Apply a 10% boost using a floating-point multiplier. The library converts it to the
    // nearest valid register value for you.
    Serial.println("Setting digital gain to 1.1x...");
    mySensor.setDigitalGainIA(1.1f);

    mySensor.getDigitalGainIA(digitalGain);
    Serial.print("Digital gain multiplier is now: ");
    Serial.println(digitalGain, 3);
    Serial.println();

    Serial.println("Reading current with new gain settings...");
    Serial.println();
}

void loop()
{
    float amps = 0.0;
    mySensor.getCurrentA(amps);

    Serial.print("Current (A): ");
    Serial.println(amps, 4);

    delay(500);
}
