/*
  Example 05 - Zero Crossing

  This example demonstrates how to configure the zero-crossing (ZX) detection
  on the ADE7953. The ZX_I pin on the board outputs a pulse each time the
  current waveform crosses zero, which is useful for determining line frequency
  and synchronizing measurements to the AC cycle.

  Configuration options shown here:
    setZXISource()  - Select Channel A or B as the ZX_I source
    setZXEdge()     - Choose which edges trigger the ZX output
    enableZXLPF()   - Enable/disable the zero-crossing low-pass filter
    getPeriod()     - Read the measured line period from the ZX detector

  The ZX_I pin on the board header can be connected to a digital input
  to count zero crossings externally.

  SparkFun Electronics
  Date: 2025
  SparkFun code, firmware, and software is released under the MIT License.
    Please see LICENSE.md for further details.

  Hardware Connections:
  IoT RedBoard --> ADE7953
  QWIIC --> QWIIC
  ZX_I pin --> Digital pin 2 (for external counting, optional)

  Serial.print it out at 115200 baud to serial monitor.

  Feel like supporting our work? Buy a board from SparkFun!
  https://www.sparkfun.com/sparkfun-qwiic-current-sensor.html
*/

#include <SparkFun_ADE7953.h>

SfeADE7953ArdI2C mySensor;

// Pin connected to the ZX_I header on the board.
// Set to -1 if not wired up — the example will still show register-based readings.
const int kZXIPin = 2;

// Zero-crossing counter for the interrupt.
volatile unsigned long zxCount = 0;

// ISR for counting zero crossings on the ZX_I pin.
void zxISR()
{
    zxCount++;
}

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("ADE7953 Example 05 - Zero Crossing");

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

    // --- Zero-crossing configuration ---

    // Select Channel A as the ZX_I source (false = Channel A, true = Channel B).
    mySensor.setZXISource(false);
    Serial.println("ZX_I source: Channel A");

    // Set edge detection to trigger on both positive and negative zero crossings.
    mySensor.setZXEdge(ADE7953_ZX_EDGE_BOTH);
    Serial.println("ZX edge: Both (positive and negative)");

    // Enable the zero-crossing low-pass filter for cleaner detection.
    mySensor.enableZXLPF(true);
    Serial.println("ZX LPF: Enabled");

    // If the ZX_I pin is wired up, attach an interrupt to count crossings.
    if (kZXIPin >= 0)
    {
        pinMode(kZXIPin, INPUT);
        attachInterrupt(digitalPinToInterrupt(kZXIPin), zxISR, RISING);
        Serial.print("External ZX_I counting on pin ");
        Serial.println(kZXIPin);
    }

    Serial.println();
    Serial.println("Reading zero-crossing data...");
    Serial.println();
}

void loop()
{
    // Read the period register. This value represents the line period
    // measured by the zero-crossing detector in units of the internal clock.
    // Period (seconds) = value / 223,750 Hz (ADE7953 ZX clock)
    uint16_t period = mySensor.getPeriod();

    Serial.print("Period register: ");
    Serial.print(period);

    if (period > 0)
    {
        // Convert to approximate line frequency.
        // The ADE7953 period register counts at 223,750 Hz (CLKIN/16),
        // and measures a full cycle (two zero crossings).
        float freqHz = 223750.0 / (float)period;
        Serial.print("  |  Line freq: ~");
        Serial.print(freqHz, 1);
        Serial.print(" Hz");
    }

    // If the ZX_I pin is wired up, show the external count.
    if (kZXIPin >= 0)
    {
        noInterrupts();
        unsigned long count = zxCount;
        interrupts();

        Serial.print("  |  ZX count: ");
        Serial.print(count);
    }

    Serial.println();
    delay(1000);
}
