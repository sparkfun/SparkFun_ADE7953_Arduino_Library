/*
  Example 08 - Channel B With Interrupt

  This example demonstrates the full Channel B path using the optional
  pinout headers on the board. It configures gain, enables Channel B
  interrupts, and reads IRMS from Channel B. This is useful when you
  want to monitor a second current source independently from the primary
  CT clamp on Channel A.

  Channel B has its own interrupt enable/status registers (IRQENB/IRQSTATB)
  which support bits [13:0]. Channel A interrupts (IRQENA/IRQSTATA) include
  additional voltage-related bits [21:14] that are not present on Channel B.

  SparkFun Electronics
  Date: 2025
  SparkFun code, firmware, and software is released under the MIT License.
    Please see LICENSE.md for further details.

  Hardware Connections:
  IoT RedBoard --> ADE7953
  QWIIC --> QWIIC
  Current source --> Channel B header pins
  IRQ pin --> Digital pin 3 (active low, optional)

  Serial.print it out at 115200 baud to serial monitor.

  Feel like supporting our work? Buy a board from SparkFun!
  https://www.sparkfun.com/sparkfun-qwiic-current-sensor.html
*/

#include <SparkFun_ADE7953.h>

SfeADE7953ArdI2C mySensor;

// Pin connected to the IRQ header on the board (active low).
// Set to -1 if not wired up — the example will poll the status register instead.
const int kIRQPin = 3;

volatile bool irqFired = false;

void irqISR()
{
    irqFired = true;
}

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("ADE7953 Example 08 - Channel B With Interrupt");

    Wire.begin();

    if (!mySensor.begin())
    {
        Serial.println("ADE7953 not found. Please check wiring. Freezing...");
        while (1)
            delay(1000);
    }

    Serial.println("ADE7953 connected!");

    // --- Channel B gain configuration ---
    // Set PGA gain to 4x on Channel B for the 5.6 ohm shunt.
    mySensor.setGainIB(ADE7953_PGA_GAIN_4);

    sfe_ade7953_pga_gain_t gainB = mySensor.getGainIB();
    Serial.print("Channel B PGA gain set to: ");
    Serial.print(gainB == ADE7953_PGA_GAIN_4 ? "4x" : "other");
    Serial.println();

    // --- Channel B interrupt configuration ---
    // Clear any pending Channel B interrupt status.
    mySensor.readAndResetInterruptStatusB();

    // Enable overcurrent and zero-crossing interrupts on Channel B.
    uint32_t irqMask = ksfADE7953IrqOI | ksfADE7953IrqZXI;
    mySensor.setInterruptEnableB(irqMask);

    // Read back to confirm.
    uint32_t irqEnabled = mySensor.getInterruptEnableB();
    Serial.print("Channel B interrupts enabled: 0x");
    Serial.println(irqEnabled, HEX);

    // Set an overcurrent threshold for Channel B.
    mySensor.setOvercurrentLevel(0x100000);
    Serial.println("Overcurrent threshold set.");

    // Set up external IRQ pin if wired.
    if (kIRQPin >= 0)
    {
        pinMode(kIRQPin, INPUT_PULLUP);
        attachInterrupt(digitalPinToInterrupt(kIRQPin), irqISR, FALLING);
        Serial.print("IRQ monitoring on pin ");
        Serial.println(kIRQPin);
    }

    // Also read Channel A alongside for comparison.
    mySensor.setGainIA(ADE7953_PGA_GAIN_4);

    Serial.println();
    Serial.println("Reading both channels...");
    Serial.println("Channel A (raw)\tChannel B (raw)\tB Interrupt");
    Serial.println("---------------\t---------------\t-----------");
}

void loop()
{
    // Read IRMS from both channels.
    uint32_t irmsA = mySensor.getIRmsA();
    uint32_t irmsB = mySensor.getIRmsB();

    Serial.print(irmsA);
    Serial.print("\t\t");
    Serial.print(irmsB);
    Serial.print("\t\t");

    // Check for Channel B interrupts via the ISR flag or by polling.
    bool hasInterrupt = false;

    if (kIRQPin >= 0 && irqFired)
    {
        irqFired = false;
        hasInterrupt = true;
    }
    else if (kIRQPin < 0)
    {
        // No IRQ pin wired — poll the status register directly.
        uint32_t status = mySensor.getInterruptStatusB();
        if (status != 0)
            hasInterrupt = true;
    }

    if (hasInterrupt)
    {
        // Read and clear Channel B interrupt status.
        uint32_t statusB = mySensor.readAndResetInterruptStatusB();

        Serial.print("0x");
        Serial.print(statusB, HEX);

        if (statusB & ksfADE7953IrqOI)
            Serial.print(" [OC]");
        if (statusB & ksfADE7953IrqZXI)
            Serial.print(" [ZX]");
    }
    else
    {
        Serial.print("--");
    }

    Serial.println();
    delay(500);
}
