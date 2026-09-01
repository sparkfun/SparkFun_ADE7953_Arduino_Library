/*
  Example 08 - Diagnostic Tool

  Interactive diagnostic sketch for the ADE7953 current sensor. Provides serial commands to
  change PGA gain, toggle the high-pass filter, perform a software zero, and view raw,
  averaged, and amps readings.

  The amps conversion is handled by the library (getCurrentA), which reads the active PGA
  gain on each call, so the displayed current stays correct no matter which gain you select.

  Serial Commands (send via Serial Monitor, line ending = Newline):
    g1    - Set PGA gain to 1x
    g2    - Set PGA gain to 2x
    g3    - Set PGA gain to 4x
    g4    - Set PGA gain to 8x
    g5    - Set PGA gain to 16x (default)
    z     - Zero calibration (averages 50 samples as a software baseline)
    r     - Reset zero offset to 0
    h     - Toggle HPF (high-pass filter) on/off
    c     - Show chip config and status
    ?     - Show help

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

// ========================= State =================================
const int AVERAGE_WINDOW = 20;       // Rolling average sample count
const int SAMPLE_INTERVAL_MS = 100;  // Milliseconds between reads

uint32_t irmsBuffer[AVERAGE_WINDOW]; // Rolling buffer of raw IRMS values
int bufferIndex = 0;
bool bufferFull = false;

uint32_t zeroOffset = 0; // Software zero offset (raw IRMS counts)

// ========================= Helpers ===============================

// Add a sample to the rolling average buffer and return the average.
uint32_t updateAverage(uint32_t sample)
{
    irmsBuffer[bufferIndex] = sample;
    bufferIndex++;
    if (bufferIndex >= AVERAGE_WINDOW)
    {
        bufferIndex = 0;
        bufferFull = true;
    }

    int count = bufferFull ? AVERAGE_WINDOW : bufferIndex;
    uint64_t sum = 0;
    for (int i = 0; i < count; i++)
        sum += irmsBuffer[i];

    return (count > 0) ? (uint32_t)(sum / count) : 0;
}

void resetBuffer()
{
    bufferIndex = 0;
    bufferFull = false;
    for (int i = 0; i < AVERAGE_WINDOW; i++)
        irmsBuffer[i] = 0;
}

// Convert a PGA gain enum to its numeric multiplier for display.
int pgaMultiplier(sfe_ade7953_pga_gain_t gain)
{
    switch (gain)
    {
    case ADE7953_PGA_GAIN_1:
        return 1;
    case ADE7953_PGA_GAIN_2:
        return 2;
    case ADE7953_PGA_GAIN_4:
        return 4;
    case ADE7953_PGA_GAIN_8:
        return 8;
    case ADE7953_PGA_GAIN_16:
        return 16;
    case ADE7953_PGA_GAIN_22:
        return 22;
    default:
        return -1;
    }
}

// ========================= Commands ==============================

void printHelp()
{
    Serial.println(F("\n===== ADE7953 Diagnostic Commands ====="));
    Serial.println(F("  g1    PGA gain 1x"));
    Serial.println(F("  g2    PGA gain 2x"));
    Serial.println(F("  g3    PGA gain 4x"));
    Serial.println(F("  g4    PGA gain 8x"));
    Serial.println(F("  g5    PGA gain 16x (default)"));
    Serial.println(F("  z     Zero calibration"));
    Serial.println(F("  r     Reset zero offset"));
    Serial.println(F("  h     Toggle HPF on/off"));
    Serial.println(F("  c     Show config / status"));
    Serial.println(F("  ?     This help"));
    Serial.println(F("=======================================\n"));
}

void setGain(sfe_ade7953_pga_gain_t gain, const char *label)
{
    if (mySensor.setGainIA(gain) != ksfTkErrOk)
    {
        Serial.println(F("Failed to set gain!"));
        return;
    }

    zeroOffset = 0;
    resetBuffer();

    Serial.print(F("PGA gain set to "));
    Serial.print(label);
    Serial.println(F("x  (offset reset, buffer cleared)"));
}

void doZeroCalibration()
{
    const int NUM_SAMPLES = 50;

    Serial.println(F("\nZero calibration - keep CT clamped with NO current flowing."));

    uint64_t sum = 0;
    for (int i = 0; i < NUM_SAMPLES; i++)
    {
        uint32_t sample = 0;
        mySensor.getIRMSA(sample);
        sum += sample;
        delay(50);
    }

    zeroOffset = (uint32_t)(sum / NUM_SAMPLES);
    resetBuffer();

    Serial.print(F("Zero offset set to "));
    Serial.print(zeroOffset);
    Serial.println(F(". Readings will subtract this baseline.\n"));
}

void showConfig()
{
    Serial.println(F("\n----- ADE7953 Status -----"));

    uint8_t version = 0;
    mySensor.getVersion(version);
    Serial.print(F("  Silicon version : 0x"));
    Serial.println(version, HEX);

    sfe_ade7953_pga_gain_t gain;
    mySensor.getGainIA(gain);
    Serial.print(F("  PGA Gain IA     : "));
    Serial.print(pgaMultiplier(gain));
    Serial.println(F("x"));

    bool hpf = false;
    mySensor.isHPFEnabled(hpf);
    Serial.print(F("  HPF enabled     : "));
    Serial.println(hpf ? "YES" : "NO");

    Serial.print(F("  Software offset : "));
    Serial.println(zeroOffset);

    Serial.println(F("--------------------------\n"));
}

void toggleHPF()
{
    bool currently = false;
    mySensor.isHPFEnabled(currently);

    if (mySensor.enableHPF(!currently) != ksfTkErrOk)
    {
        Serial.println(F("Failed to toggle HPF!"));
        return;
    }

    resetBuffer();
    Serial.print(F("HPF "));
    Serial.println(!currently ? F("ENABLED") : F("DISABLED"));
}

void processCommand()
{
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd == "g1")
        setGain(ADE7953_PGA_GAIN_1, "1");
    else if (cmd == "g2")
        setGain(ADE7953_PGA_GAIN_2, "2");
    else if (cmd == "g3")
        setGain(ADE7953_PGA_GAIN_4, "4");
    else if (cmd == "g4")
        setGain(ADE7953_PGA_GAIN_8, "8");
    else if (cmd == "g5")
        setGain(ADE7953_PGA_GAIN_16, "16");
    else if (cmd == "z")
        doZeroCalibration();
    else if (cmd == "r")
    {
        zeroOffset = 0;
        resetBuffer();
        Serial.println(F("Offset reset to 0, buffer cleared."));
    }
    else if (cmd == "h")
        toggleHPF();
    else if (cmd == "c")
        showConfig();
    else if (cmd == "?")
        printHelp();
    else
    {
        Serial.print(F("Unknown command: "));
        Serial.println(cmd);
        printHelp();
    }
}

// ========================= Setup / Loop ==========================

void setup()
{
    Serial.begin(115200);

    Serial.println(F("\n========================================"));
    Serial.println(F("  ADE7953 Diagnostic Tool"));
    Serial.println(F("========================================"));

    Wire.begin();

    while (mySensor.begin() == false)
    {
        Serial.println(F("ADE7953 not connected, check your wiring!"));
        delay(1000);
    }

    Serial.println(F("ADE7953 connected!"));

    resetBuffer();
    showConfig();
    printHelp();
}

void loop()
{
    if (Serial.available())
        processCommand();

    // Read raw IRMS from Channel A.
    uint32_t rawIrms = 0;
    mySensor.getIRMSA(rawIrms);

    // Apply the software zero offset (clamped at zero).
    uint32_t corrected = (rawIrms > zeroOffset) ? (rawIrms - zeroOffset) : 0;

    // Rolling average of the corrected values.
    uint32_t averaged = updateAverage(corrected);

    // Instantaneous current - signed, shows DC offset directly.
    int32_t instantIA = 0;
    mySensor.getInstantaneousIA(instantIA);

    // Real-world current in amps (library handles the gain/CT/burden conversion).
    float amps = 0.0;
    mySensor.getCurrentA(amps);

    Serial.print(F("Raw: "));
    Serial.print(rawIrms);
    Serial.print(F(" | Corrected: "));
    Serial.print(corrected);
    Serial.print(F(" | Averaged: "));
    Serial.print(averaged);
    Serial.print(F(" | Amps: "));
    Serial.print(amps, 4);
    Serial.print(F(" | Inst IA: "));
    Serial.println(instantIA);

    delay(SAMPLE_INTERVAL_MS);
}
