/*
  Example 07 - Auto Calibration & Error Handling

  Two things are shown here:

  1. Auto calibration. Even with no current flowing, the ADC reports a small nonzero IRMS
     value due to noise. autoCalibrateA() averages a number of no-load samples and stores
     that baseline; getCurrentA() then removes it in the squared domain
     (corrected = sqrt(reading^2 - baseline^2)), which is the physically correct way to
     subtract an RMS noise floor and needs no datasheet-specific scaling. The hardware
     AIRMSOS register is left untouched, but setIRMSOffsetA()/getIRMSOffsetA() are still
     available if you want to use it.

  2. Error handling. Every library read/write returns a SparkFun Toolkit error code
     (ksfTkErrOk on success, a negative value on failure). Most sketches ignore it for
     simplicity, but this example checks it so you can detect a misbehaving device.

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
const uint16_t kCalSamples = 50;

void setup()
{
    Serial.begin(115200);
    Serial.println("SparkFun ADE7953 Example 7 - Auto Calibration & Error Handling");

    Wire.begin();

    while (mySensor.begin() == false)
    {
        Serial.println("ADE7953 not connected, check your wiring!");
        delay(1000);
    }

    Serial.println("ADE7953 connected!");

    // --- Calibration phase ---
    Serial.println();
    Serial.println("=== CALIBRATION PHASE ===");
    Serial.println("Ensure NO load is connected to Channel A.");
    delay(1000);

    // Average kCalSamples no-load readings into the channel's software baseline. Check the
    // return code so a communication failure is not silently ignored.
    if (mySensor.autoCalibrateA(kCalSamples) != ksfTkErrOk)
        Serial.println("Calibration failed - check the connection!");
    else
        Serial.println("Calibration complete. Baseline noise will be removed.");

    Serial.println("You may now connect a load.");
    Serial.println();
}

void loop()
{
    // getCurrentA() applies the calibrated baseline automatically. Check the return code so a
    // communication error is not mistaken for a zero reading.
    float amps = 0.0;
    if (mySensor.getCurrentA(amps) != ksfTkErrOk)
    {
        Serial.println("Current read failed!");
        delay(500);
        return;
    }

    Serial.print("Calibrated current (A): ");
    Serial.println(amps, 4);

    delay(500);
}
