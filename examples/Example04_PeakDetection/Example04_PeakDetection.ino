/*
  Example 04 - Peak Detection

  This example demonstrates how to read peak current values from Channel A.
  The ADE7953 continuously tracks the highest instantaneous current sample
  since the last reset. This is useful for detecting transient events like
  motor startup surges or inrush current.

  Two methods are available:
    getPeakIA()           - Reads the peak value (does NOT clear it)
    readAndResetPeakIA()  - Reads the peak value AND clears it atomically

  This example uses readAndResetPeakIA() to track the peak current over
  fixed intervals.

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

// Interval in milliseconds between peak resets.
const unsigned long kPeakInterval = 2000;
unsigned long lastResetTime = 0;

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("ADE7953 Example 04 - Peak Detection");

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

    Serial.println("Reading peak current every 2 seconds...");
    Serial.println();

    // Clear any stale peak value by doing an initial read-and-reset.
    mySensor.readAndResetPeakIA();
    lastResetTime = millis();
}

void loop()
{
    // Also print the current IRMS for context.
    uint32_t irms = mySensor.getIRmsA();

    Serial.print("IRMS: ");
    Serial.print(irms);

    // Check the running peak (non-destructive read).
    uint32_t runningPeak = mySensor.getPeakIA();
    Serial.print("  |  Running Peak: ");
    Serial.print(runningPeak);

    // Every kPeakInterval milliseconds, atomically read and reset the peak.
    if ((millis() - lastResetTime) >= kPeakInterval)
    {
        uint32_t peak = mySensor.readAndResetPeakIA();

        Serial.print("  |  ** Peak (reset): ");
        Serial.print(peak);
        Serial.print(" **");

        lastResetTime = millis();
    }

    Serial.println();
    delay(250);
}
