/*
  Example 05 - Zero Crossing

  Configures the zero-crossing (ZX) detection on the ADE7953. The ZX_I pin on the board
  outputs a pulse each time the current waveform crosses zero, which is useful for
  determining line frequency and synchronizing measurements to the AC cycle.

  Configuration shown here:
    setZXISourceChannel() - Select Channel A or B as the ZX_I source
    setZXEdge()           - Choose which edges trigger the ZX output
    enableZXLPF()         - Enable/disable the zero-crossing low-pass filter
    getPeriod()           - Read the measured line period from the ZX detector

  SparkFun Electronics
  Date: 2025
  SparkFun code, firmware, and software is released under the MIT License.
    Please see LICENSE.md for further details.

  Hardware Connections:
  IoT RedBoard --> ADE7953
  QWIIC --> QWIIC
  ZX_I pin --> Digital pin 2 (for external counting, optional)

  Open the Serial Monitor at 115200 baud.

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

void zxISR()
{
    zxCount++;
}

void setup()
{
    Serial.begin(115200);
    Serial.println("SparkFun ADE7953 Example 5 - Zero Crossing");

    Wire.begin();

    while (mySensor.begin() == false)
    {
        Serial.println("ADE7953 not connected, check your wiring!");
        delay(1000);
    }

    Serial.println("ADE7953 connected!");

    // --- Zero-crossing configuration ---

    // Select Channel A as the ZX_I source (false = Channel A, true = Channel B).
    mySensor.setZXISourceChannel(false);
    Serial.println("ZX_I source: Channel A");

    // Trigger on both positive and negative zero crossings.
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
    // Read the period register. This value represents the line period measured by the
    // zero-crossing detector in units of the internal clock.
    uint16_t period = 0;
    mySensor.getPeriod(period);

    Serial.print("Period register: ");
    Serial.print(period);

    if (period > 0)
    {
        // Convert to approximate line frequency. The ADE7953 period register counts at
        // 223,750 Hz and measures a full cycle.
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
