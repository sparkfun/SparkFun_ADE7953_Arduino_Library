/*
  Example 04 - Peak Detection

  Reads peak current values from Channel A. The ADE7953 continuously tracks the highest
  instantaneous current sample since the last reset, which is useful for detecting transient
  events like motor startup surges or inrush current.

  Two methods are available:
    getPeakIA()           - Reads the running peak (does NOT clear it)
    readAndResetPeakIA()  - Reads the peak value AND clears it in a single operation

  This example reports the peak current over fixed two-second intervals.

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

// Interval in milliseconds between peak resets.
const unsigned long kPeakInterval = 2000;
unsigned long lastResetTime = 0;

void setup()
{
    Serial.begin(115200);
    Serial.println("SparkFun ADE7953 Example 4 - Peak Detection");

    Wire.begin();

    while (mySensor.begin() == false)
    {
        Serial.println("ADE7953 not connected, check your wiring!");
        delay(1000);
    }

    Serial.println("ADE7953 connected!");
    Serial.println("Reading peak current every 2 seconds...");
    Serial.println();

    // Clear any stale peak value with an initial read-and-reset.
    uint32_t discard = 0;
    mySensor.readAndResetPeakIA(discard);
    lastResetTime = millis();
}

void loop()
{
    // Print the current IRMS value (raw) for context.
    uint32_t irms = 0;
    mySensor.getIRMSA(irms);
    Serial.print("IRMS (raw): ");
    Serial.print(irms);

    // Check the running peak (non-destructive read).
    uint32_t runningPeak = 0;
    mySensor.getPeakIA(runningPeak);
    Serial.print("  |  Running Peak: ");
    Serial.print(runningPeak);

    // Every interval, read and clear the peak.
    if ((millis() - lastResetTime) >= kPeakInterval)
    {
        uint32_t peak = 0;
        mySensor.readAndResetPeakIA(peak);

        Serial.print("  |  ** Peak (reset): ");
        Serial.print(peak);
        Serial.print(" **");

        lastResetTime = millis();
    }

    Serial.println();
    delay(250);
}
