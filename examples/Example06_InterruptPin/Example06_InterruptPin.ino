/*
  Example 06 - Interrupt Pin

  This example demonstrates how to configure and use the ADE7953 interrupt
  system. The IRQ pin on the board header is active low and can be connected
  to a GPIO to detect events like overcurrent, zero-crossing, or cycle
  completion without polling.

  This example sets up an overcurrent threshold on Channel A and enables
  the overcurrent interrupt. When the threshold is exceeded, the IRQ pin
  goes low and the interrupt status register reports which event fired.

  Interrupt bitmask constants (partial list):
    ksfADE7953IrqOI      - Overcurrent detected
    ksfADE7953IrqZXI     - Current zero-crossing detected
    ksfADE7953IrqCycEnd  - Line cycle accumulation complete
    ksfADE7953IrqSag     - Sag event occurred
    ksfADE7953IrqReset   - Reset occurred (always enabled)

  SparkFun Electronics
  Date: 2025
  SparkFun code, firmware, and software is released under the MIT License.
    Please see LICENSE.md for further details.

  Hardware Connections:
  IoT RedBoard --> ADE7953
  QWIIC --> QWIIC
  IRQ pin --> Digital pin 3 (active low)

  Serial.print it out at 115200 baud to serial monitor.

  Feel like supporting our work? Buy a board from SparkFun!
  https://www.sparkfun.com/sparkfun-qwiic-current-sensor.html
*/

#include <SparkFun_ADE7953.h>

SfeADE7953ArdI2C mySensor;

// Pin connected to the IRQ header on the board (active low).
const int kIRQPin = 3;

// Flag set by the ISR when the IRQ pin goes low.
volatile bool irqFired = false;

void irqISR()
{
    irqFired = true;
}

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("ADE7953 Example 06 - Interrupt Pin");

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

    // --- Interrupt configuration ---

    // Set an overcurrent threshold. This value is in raw ADC counts.
    // Adjust this based on your expected current range.
    uint32_t overcurrentThreshold = 0x100000;
    mySensor.setOvercurrentLevel(overcurrentThreshold);
    Serial.print("Overcurrent threshold set to: 0x");
    Serial.println(overcurrentThreshold, HEX);

    // Clear any pending interrupt status by reading and resetting.
    mySensor.readAndResetInterruptStatusA();

    // Enable the overcurrent interrupt on Channel A.
    // You can OR multiple bitmasks together to enable several interrupts:
    //   ksfADE7953IrqOI | ksfADE7953IrqZXI | ksfADE7953IrqCycEnd
    mySensor.setInterruptEnableA(ksfADE7953IrqOI);
    Serial.println("Overcurrent interrupt enabled on Channel A.");

    // Set up the external IRQ pin (active low, so trigger on FALLING).
    pinMode(kIRQPin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(kIRQPin), irqISR, FALLING);
    Serial.print("IRQ monitoring on pin ");
    Serial.println(kIRQPin);

    Serial.println();
    Serial.println("Waiting for interrupts...");
    Serial.println();
}

void loop()
{
    // Print the current IRMS value for context.
    uint32_t irms = mySensor.getIRmsA();
    Serial.print("IRMS: ");
    Serial.print(irms);

    // Check if the ISR flagged an interrupt.
    if (irqFired)
    {
        irqFired = false;

        // Read and clear the interrupt status register.
        uint32_t status = mySensor.readAndResetInterruptStatusA();

        Serial.print("  ** IRQ! Status: 0x");
        Serial.print(status, HEX);

        // Decode which interrupt(s) fired.
        if (status & ksfADE7953IrqOI)
            Serial.print(" [Overcurrent]");
        if (status & ksfADE7953IrqZXI)
            Serial.print(" [ZeroCrossing]");
        if (status & ksfADE7953IrqCycEnd)
            Serial.print(" [CycleEnd]");
        if (status & ksfADE7953IrqSag)
            Serial.print(" [Sag]");
        if (status & ksfADE7953IrqReset)
            Serial.print(" [Reset]");

        Serial.print(" **");
    }

    Serial.println();
    delay(500);
}
