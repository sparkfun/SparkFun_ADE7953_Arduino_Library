/*
  Example 07 - Calibration Offset & Error Handling

  Two things are shown here:

  1. Error handling. Every library read/write returns a SparkFun Toolkit error code
     (ksfTkErrOk on success, a negative value on failure). Most sketches ignore it for
     simplicity, but this example shows how to check it so you can detect a disconnected
     or misbehaving device.

  2. Zeroing the no-load baseline. Even with no current flowing, the ADC reports a small
     nonzero IRMS value due to noise. This example measures that baseline and removes it.

  A note on the hardware offset register (AIRMSOS): the ADE7953 applies it in the squared
  domain (the offset is added to the square of the RMS result before the square root), so
  the value needed to null a noise floor of N counts is NOT simply -N. The exact scaling is
  datasheet dependent. To keep this example correct and easy to follow, the baseline is
  removed in software; getIRmsOffsetA()/setIRmsOffsetA() are demonstrated so you can apply a
  hardware offset once you have determined the right value for your setup.

  Run with NO load connected to Channel A during the calibration phase.

  SparkFun Electronics
  Date: 2025
  SparkFun code, firmware, and software is released under the MIT License.
    Please see LICENSE.md for further details.

  Hardware Connections:
  IoT RedBoard --> ADE7953
  QWIIC --> QWIIC
  ** Disconnect load from CT clamp during calibration phase **

  Open the Serial Monitor at 115200 baud.

  Feel like supporting our work? Buy a board from SparkFun!
  https://www.sparkfun.com/sparkfun-qwiic-current-sensor.html
*/

#include <SparkFun_ADE7953.h>

SfeADE7953ArdI2C mySensor;

// Number of samples to average during calibration.
const int kCalSamples = 50;

// Software baseline (raw IRMS counts measured with no load).
uint32_t baseline = 0;

void setup()
{
    Serial.begin(115200);
    Serial.println("SparkFun ADE7953 Example 7 - Calibration Offset & Error Handling");

    Wire.begin();

    while (mySensor.begin() == false)
    {
        Serial.println("ADE7953 not connected, check your wiring!");
        delay(1000);
    }

    Serial.println("ADE7953 connected!");

    // Clear any existing hardware offset so it does not affect our baseline measurement.
    mySensor.setIRmsOffsetA(0);

    int32_t hwOffset = 0;
    mySensor.getIRmsOffsetA(hwOffset);
    Serial.print("Hardware IRMS offset register: ");
    Serial.println(hwOffset);

    // --- Calibration phase: average the no-load reading ---
    Serial.println();
    Serial.println("=== CALIBRATION PHASE ===");
    Serial.println("Ensure NO load is connected to Channel A.");
    delay(1000);

    // Use a 64-bit accumulator so the sum cannot overflow regardless of sample count.
    uint64_t sum = 0;
    for (int i = 0; i < kCalSamples; i++)
    {
        uint32_t sample = 0;

        // Demonstrate error checking: only use the sample if the read succeeded.
        if (mySensor.getIRmsA(sample) != ksfTkErrOk)
        {
            Serial.println("Read failed during calibration!");
            i--; // retry this sample
            delay(100);
            continue;
        }

        sum += sample;
        delay(100);
    }

    baseline = (uint32_t)(sum / kCalSamples);
    Serial.print("No-load baseline (raw counts): ");
    Serial.println(baseline);
    Serial.println("=== CALIBRATION COMPLETE ===");
    Serial.println("You may now connect a load. Readings have the baseline removed.");
    Serial.println();
}

void loop()
{
    uint32_t raw = 0;

    // Check the return code so a communication error is not mistaken for a zero reading.
    if (mySensor.getIRmsA(raw) != ksfTkErrOk)
    {
        Serial.println("IRMS read failed!");
        delay(500);
        return;
    }

    // Remove the no-load baseline in software (clamped at zero).
    uint32_t corrected = (raw > baseline) ? (raw - baseline) : 0;

    Serial.print("Raw: ");
    Serial.print(raw);
    Serial.print("  |  Baseline-corrected: ");
    Serial.println(corrected);

    delay(500);
}
