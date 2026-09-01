/*
  Example 06 - Interrupt Pin

  Configures and uses the ADE7953 interrupt system. The IRQ pin on the board header is
  active low and can be connected to a GPIO to detect events like overcurrent,
  zero-crossing, or cycle completion without polling.

  This example sets an overcurrent threshold on Channel A and enables the overcurrent
  interrupt. When the threshold is exceeded, the IRQ pin goes low and the interrupt status
  register reports which event fired.

  Interrupts are configured with the sfe_ade7953_irq_reg_t bitfield. Set the field for the
  interrupt(s) you want, for example: irqEnable.oI = 1 (overcurrent), irqEnable.zxI = 1
  (zero-crossing), irqEnable.cycEnd = 1 (line cycle complete).

  SparkFun Electronics
  Date: 2025
  SparkFun code, firmware, and software is released under the MIT License.
    Please see LICENSE.md for further details.

  Hardware Connections:
  IoT RedBoard --> ADE7953
  QWIIC --> QWIIC
  IRQ pin --> Digital pin 3 (active low)

  Open the Serial Monitor at 115200 baud.

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
    Serial.println("SparkFun ADE7953 Example 6 - Interrupt Pin");

    Wire.begin();

    while (mySensor.begin() == false)
    {
        Serial.println("ADE7953 not connected, check your wiring!");
        delay(1000);
    }

    Serial.println("ADE7953 connected!");

    // --- Interrupt configuration ---

    // Set an overcurrent threshold (raw ADC counts). Adjust for your expected current range.
    uint32_t overcurrentThreshold = 0x100000;
    mySensor.setOvercurrentLevel(overcurrentThreshold);
    Serial.print("Overcurrent threshold set to: 0x");
    Serial.println(overcurrentThreshold, HEX);

    // Clear any pending interrupt status.
    sfe_ade7953_irq_reg_t irqStatus = {};
    mySensor.readAndResetInterruptStatusA(irqStatus);

    // Enable the overcurrent interrupt on Channel A.
    sfe_ade7953_irq_reg_t irqEnable = {};
    irqEnable.oI = 1;
    mySensor.setInterruptEnableA(irqEnable);
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
    // Print the current IRMS value (raw) for context.
    uint32_t irms = 0;
    mySensor.getIRMSA(irms);
    Serial.print("IRMS (raw): ");
    Serial.print(irms);

    // Check if the ISR flagged an interrupt.
    if (irqFired)
    {
        irqFired = false;

        // Read and clear the interrupt status register.
        sfe_ade7953_irq_reg_t status = {};
        mySensor.readAndResetInterruptStatusA(status);

        Serial.print("  ** IRQ! Status: 0x");
        Serial.print(status.word, HEX);

        // Decode which interrupt(s) fired.
        if (status.oI)
            Serial.print(" [Overcurrent]");
        if (status.zxI)
            Serial.print(" [ZeroCrossing]");
        if (status.cycEnd)
            Serial.print(" [CycleEnd]");
        if (status.sag)
            Serial.print(" [Sag]");
        if (status.reset)
            Serial.print(" [Reset]");

        Serial.print(" **");
    }

    Serial.println();
    delay(500);
}
