/*
  Example 07 - Calibration Offset

  This example demonstrates how to use the IRMS offset register to zero out
  no-load readings on Channel A. Even with no current flowing, the ADC may
  report a small nonzero IRMS value due to noise. The offset register lets
  you subtract that baseline for more accurate low-current measurements.

  How it works:
    1. Read several IRMS samples with no load connected.
    2. Average them to find the baseline noise floor.
    3. Write the negative of that average into the IRMS offset register.
    4. Future IRMS readings will have the baseline subtracted automatically.

  Note: Run this example with NO load connected to Channel A during the
  calibration phase (the first few seconds after startup).

  SparkFun Electronics
  Date: 2025
  SparkFun code, firmware, and software is released under the MIT License.
    Please see LICENSE.md for further details.

  Hardware Connections:
  IoT RedBoard --> ADE7953
  QWIIC --> QWIIC
  ** Disconnect load from CT clamp during calibration phase **

  Serial.print it out at 115200 baud to serial monitor.

  Feel like supporting our work? Buy a board from SparkFun!
  https://www.sparkfun.com/sparkfun-qwiic-current-sensor.html
*/

#include <SparkFun_ADE7953.h>

SfeADE7953ArdI2C mySensor;

// Number of samples to average during calibration.
const int kCalSamples = 20;

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("ADE7953 Example 07 - Calibration Offset");

    Wire.begin();

    if (!mySensor.begin())
    {
        Serial.println("ADE7953 not found. Please check wiring. Freezing...");
        while (1)
            delay(1000);
    }

    Serial.println("ADE7953 connected!");

    // Set PGA gain to 4x for the 5.6 ohm shunt resistor.
    mySensor.setGainIA(ADE7953_PGA_GAIN_4);

    // Clear any existing offset before calibrating.
    mySensor.setIRmsOffsetA(0);

    // Read the current offset to confirm it is zero.
    int32_t currentOffset = mySensor.getIRmsOffsetA();
    Serial.print("Current IRMS offset: ");
    Serial.println(currentOffset);

    // --- Calibration phase ---
    // Make sure NO load is connected to the CT clamp during this phase!
    Serial.println();
    Serial.println("=== CALIBRATION PHASE ===");
    Serial.println("Ensure no load is connected to Channel A.");
    Serial.print("Averaging ");
    Serial.print(kCalSamples);
    Serial.println(" samples...");
    Serial.println();

    // Let the readings stabilize.
    delay(1000);

    // Collect samples and compute the average baseline.
    uint32_t sum = 0;
    for (int i = 0; i < kCalSamples; i++)
    {
        uint32_t sample = mySensor.getIRmsA();
        Serial.print("  Sample ");
        Serial.print(i + 1);
        Serial.print(": ");
        Serial.println(sample);
        sum += sample;
        delay(100);
    }

    uint32_t average = sum / kCalSamples;
    Serial.println();
    Serial.print("Average no-load reading: ");
    Serial.println(average);

    // Write the negative of the average as the offset.
    // The ADE7953 subtracts the offset from the squared signal before
    // taking the square root, so this effectively removes the noise floor.
    int32_t offset = -((int32_t)average);
    Serial.print("Setting IRMS offset to: ");
    Serial.println(offset);
    mySensor.setIRmsOffsetA(offset);

    // Read back to confirm.
    currentOffset = mySensor.getIRmsOffsetA();
    Serial.print("IRMS offset is now: ");
    Serial.println(currentOffset);

    Serial.println();
    Serial.println("=== CALIBRATION COMPLETE ===");
    Serial.println("You may now connect a load. Reading calibrated current...");
    Serial.println();
}

void loop()
{
    uint32_t irms = mySensor.getIRmsA();

    Serial.print("IRMS Channel A (calibrated): ");
    Serial.println(irms);

    delay(500);
}
