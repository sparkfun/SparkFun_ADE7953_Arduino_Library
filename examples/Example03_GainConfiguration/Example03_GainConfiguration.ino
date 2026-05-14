/*
  Example 03 - Gain Configuration

  This example demonstrates how to configure the PGA (Programmable Gain
  Amplifier) and digital gain on Channel A. The PGA gain should be set
  based on the shunt resistor value on the board:
    - 5.6 ohm shunt: use 4x gain (ADE7953_PGA_GAIN_4)

  Available PGA gain settings:
    ADE7953_PGA_GAIN_1   (1x)
    ADE7953_PGA_GAIN_2   (2x)
    ADE7953_PGA_GAIN_4   (4x)  <-- default for this board
    ADE7953_PGA_GAIN_8   (8x)
    ADE7953_PGA_GAIN_16  (16x)
    ADE7953_PGA_GAIN_22  (22x, current channels only)

  The digital gain register (AIGAIN) provides fine-tuning on top of the PGA.
  Its default value is 0x400000 (unity). Values above this increase gain,
  values below decrease it.

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
    delay(1000);
    Serial.println("ADE7953 Example 03 - Gain Configuration");

    Wire.begin();

    if (!mySensor.begin())
    {
        Serial.println("ADE7953 not found. Please check wiring. Freezing...");
        while (1)
            delay(1000);
    }

    Serial.println("ADE7953 connected!");
    Serial.println();

    // Read and display the default PGA gain for Channel A.
    sfe_ade7953_pga_gain_t currentGain = mySensor.getGainIA();
    Serial.print("Default PGA gain (Channel A): ");
    printGain(currentGain);
    Serial.println();

    // Set the PGA gain to 4x for the 5.6 ohm shunt on this board.
    Serial.println("Setting PGA gain to 4x...");
    mySensor.setGainIA(ADE7953_PGA_GAIN_4);

    // Read back to confirm.
    currentGain = mySensor.getGainIA();
    Serial.print("PGA gain is now: ");
    printGain(currentGain);
    Serial.println();
    Serial.println();

    // Read the default digital gain.
    uint32_t digitalGain = mySensor.getDigitalGainIA();
    Serial.print("Default digital gain (AIGAIN): 0x");
    Serial.println(digitalGain, HEX);

    // Set a custom digital gain (10% increase above unity 0x400000).
    uint32_t customDigitalGain = 0x466666; // ~1.1x multiplier
    Serial.print("Setting digital gain to: 0x");
    Serial.println(customDigitalGain, HEX);
    mySensor.setDigitalGainIA(customDigitalGain);

    // Read back to confirm.
    digitalGain = mySensor.getDigitalGainIA();
    Serial.print("Digital gain is now: 0x");
    Serial.println(digitalGain, HEX);
    Serial.println();

    Serial.println("Reading current with new gain settings...");
    Serial.println();
}

void loop()
{
    uint32_t irms = mySensor.getIRmsA();

    Serial.print("IRMS Channel A (raw): ");
    Serial.println(irms);

    delay(500);
}
