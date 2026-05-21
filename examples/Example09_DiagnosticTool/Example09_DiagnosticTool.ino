/*
  Example 09 - Diagnostic Tool

  Interactive diagnostic sketch for the ADE7953 current sensor. Provides
  serial commands to change PGA gain, toggle the high-pass filter, perform
  zero calibration, and view averaged/raw current readings with amps conversion.

  Prints both raw IRMS and instantaneous IA (signed) so you can see DC offset.
  Includes a rolling average to smooth out noisy readings.

  Serial Commands (send via Serial Monitor, line ending = Newline):
    g1    - Set PGA gain to 1x
    g2    - Set PGA gain to 2x
    g3    - Set PGA gain to 4x  (default)
    g4    - Set PGA gain to 8x
    g5    - Set PGA gain to 16x
    z     - Zero calibration (averages 50 samples as baseline offset)
    r     - Reset zero offset to 0
    h     - Toggle HPF (high-pass filter) on/off
    c     - Show chip config and status
    p     - Toggle plotter mode (CSV output for Serial Plotter)
    ?     - Show help

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

// ========================= Configuration ==========================
// Modify these for your CT / burden resistor setup.
const float CT_RATIO = 2000.0;         // CT turns ratio (30A:15mA = 2000:1)
const float BURDEN_RESISTOR = 5.6;     // Burden resistor in ohms
const float FULL_SCALE_CODE = 5928256.0; // ADE7953 full-scale IRMS code
const float FULL_SCALE_VRMS = 0.35355; // 0.5V peak / sqrt(2)

const int AVERAGE_WINDOW = 20;         // Rolling average sample count
const int SAMPLE_INTERVAL_MS = 100;    // Milliseconds between reads

// ========================= State =================================
uint32_t irmsBuffer[AVERAGE_WINDOW];
int bufferIndex = 0;
bool bufferFull = false;

int32_t zeroOffset = 0;                // Software zero offset (raw IRMS units)
float pgaMultiplier = 4.0;             // Tracks current PGA setting
bool plotterMode = false;              // CSV output for Arduino Serial Plotter

// ========================= Helpers ===============================

/// @brief Convert a corrected raw IRMS value to amps.
float rawToAmps(int32_t rawValue)
{
    if (rawValue <= 0)
        return 0.0;

    // Pin voltage RMS = raw / fullScaleCode * (fullScaleVrms / pgaGain)
    // Secondary current = pinVrms / burdenResistor
    // Primary current   = secondaryCurrent * ctRatio
    float fullScaleAmps = (FULL_SCALE_VRMS / pgaMultiplier) / BURDEN_RESISTOR * CT_RATIO;
    return (float)rawValue / FULL_SCALE_CODE * fullScaleAmps;
}

/// @brief Add a sample to the rolling average buffer and return the average.
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

/// @brief Reset the rolling average buffer.
void resetBuffer()
{
    bufferIndex = 0;
    bufferFull = false;
    memset(irmsBuffer, 0, sizeof(irmsBuffer));
}

// ========================= Commands ==============================

void printHelp()
{
    Serial.println(F("\n===== ADE7953 Diagnostic Commands ====="));
    Serial.println(F("  g1    PGA gain 1x"));
    Serial.println(F("  g2    PGA gain 2x"));
    Serial.println(F("  g3    PGA gain 4x  (default)"));
    Serial.println(F("  g4    PGA gain 8x"));
    Serial.println(F("  g5    PGA gain 16x"));
    Serial.println(F("  z     Zero calibration"));
    Serial.println(F("  r     Reset zero offset"));
    Serial.println(F("  h     Toggle HPF on/off"));
    Serial.println(F("  c     Show config / status"));
    Serial.println(F("  p     Toggle plotter mode"));
    Serial.println(F("  ?     This help"));
    Serial.println(F("=======================================\n"));
}

void setGain(sfe_ade7953_pga_gain_t gain, float multiplier, const char *label)
{
    if (!mySensor.setGainIA(gain))
    {
        Serial.println(F("Failed to set gain!"));
        return;
    }

    pgaMultiplier = multiplier;
    zeroOffset = 0;
    resetBuffer();

    Serial.print(F("PGA gain set to "));
    Serial.print(label);
    Serial.println(F("x  (offset reset, buffer cleared)"));
}

void doZeroCalibration()
{
    const int NUM_SAMPLES = 50;

    Serial.println(F("\nZero calibration — keep CT clamped with NO current flowing."));
    Serial.print(F("Collecting "));
    Serial.print(NUM_SAMPLES);
    Serial.println(F(" samples..."));

    uint64_t sum = 0;
    for (int i = 0; i < NUM_SAMPLES; i++)
    {
        sum += mySensor.getIRmsA();
        delay(50);
        if ((i + 1) % 10 == 0)
        {
            Serial.print(F("  "));
            Serial.print(i + 1);
            Serial.print(F("/"));
            Serial.println(NUM_SAMPLES);
        }
    }

    zeroOffset = (int32_t)(sum / NUM_SAMPLES);
    resetBuffer();

    Serial.print(F("Zero offset set to "));
    Serial.print(zeroOffset);
    Serial.println(F(". Readings will subtract this baseline.\n"));
}

/// @brief Convert PGA gain enum to its actual multiplier for display.
int pgaEnumToMultiplier(int enumVal)
{
    switch (enumVal)
    {
    case 0:
        return 1;
    case 1:
        return 2;
    case 2:
        return 4;
    case 3:
        return 8;
    case 4:
        return 16;
    case 5:
        return 22;
    default:
        return -1;
    }
}

void showConfig()
{
    Serial.println(F("\n----- ADE7953 Status -----"));

    Serial.print(F("  Silicon version : 0x"));
    Serial.println(mySensor.getVersion(), HEX);

    Serial.print(F("  PGA Gain IA     : "));
    Serial.print(pgaEnumToMultiplier((int)mySensor.getGainIA()));
    Serial.println(F("x"));

    Serial.print(F("  PGA Gain IB     : "));
    Serial.print(pgaEnumToMultiplier((int)mySensor.getGainIB()));
    Serial.println(F("x"));

    Serial.print(F("  PGA Gain V      : "));
    Serial.print(pgaEnumToMultiplier((int)mySensor.getGainV()));
    Serial.println(F("x"));

    Serial.print(F("  HPF enabled     : "));
    Serial.println(mySensor.isHPFEnabled() ? "YES" : "NO");

    Serial.print(F("  Software offset : "));
    Serial.println(zeroOffset);

    Serial.print(F("  PGA multiplier  : "));
    Serial.print(pgaMultiplier, 1);
    Serial.println(F("x"));

    float fullScale = (FULL_SCALE_VRMS / pgaMultiplier) / BURDEN_RESISTOR * CT_RATIO;
    Serial.print(F("  Full-scale amps : "));
    Serial.print(fullScale, 2);
    Serial.println(F(" A"));

    Serial.println(F("--------------------------\n"));
}

void toggleHPF()
{
    bool currently = mySensor.isHPFEnabled();
    bool newState = !currently;

    if (!mySensor.enableHPF(newState))
    {
        Serial.println(F("Failed to toggle HPF!"));
        return;
    }

    resetBuffer();
    Serial.print(F("HPF "));
    Serial.println(newState ? F("ENABLED") : F("DISABLED"));
}

void printTableHeader()
{
    if (!plotterMode)
    {
        Serial.println();
        Serial.println(F("   ms     | Raw IRMS | Corrected | Averaged |   Amps   | Inst. IA (signed)"));
        Serial.println(F("----------|----------|-----------|----------|----------|------------------"));
    }
}

void processCommand()
{
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd == "g1")
        setGain(ADE7953_PGA_GAIN_1, 1.0, "1");
    else if (cmd == "g2")
        setGain(ADE7953_PGA_GAIN_2, 2.0, "2");
    else if (cmd == "g3")
        setGain(ADE7953_PGA_GAIN_4, 4.0, "4");
    else if (cmd == "g4")
        setGain(ADE7953_PGA_GAIN_8, 8.0, "8");
    else if (cmd == "g5")
        setGain(ADE7953_PGA_GAIN_16, 16.0, "16");
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
    else if (cmd == "p")
    {
        plotterMode = !plotterMode;
        Serial.print(F("Plotter mode "));
        Serial.println(plotterMode ? F("ON (CSV)") : F("OFF (table)"));
    }
    else if (cmd == "?")
        printHelp();
    else
    {
        Serial.print(F("Unknown command: "));
        Serial.println(cmd);
        printHelp();
    }

    printTableHeader();
}

// ========================= Setup / Loop ==========================

void setup()
{
    Serial.begin(115200);
    delay(1000);

    Serial.println(F("\n========================================"));
    Serial.println(F("  ADE7953 Diagnostic Tool"));
    Serial.println(F("========================================"));

    Wire.begin();

    if (!mySensor.begin())
    {
        Serial.println(F("ADE7953 not found. Please check wiring. Freezing..."));
        while (1)
            delay(1000);
    }

    Serial.print(F("ADE7953 connected! Silicon version: 0x"));
    Serial.println(mySensor.getVersion(), HEX);

    // Default gain: 4x for 5.6 ohm burden
    mySensor.setGainIA(ADE7953_PGA_GAIN_4);

    // Make sure HPF is on
    mySensor.enableHPF(true);

    // Let DSP settle before we start reading
    Serial.println(F("Waiting for DSP to settle..."));
    delay(1000);

    memset(irmsBuffer, 0, sizeof(irmsBuffer));

    showConfig();
    printHelp();
    printTableHeader();
}

void loop()
{
    if (Serial.available())
        processCommand();

    // Read raw IRMS from Channel A
    uint32_t rawIrms = mySensor.getIRmsA();

    // Apply software zero offset
    int32_t corrected = (int32_t)rawIrms - zeroOffset;
    if (corrected < 0)
        corrected = 0;

    // Rolling average of corrected values
    uint32_t averaged = updateAverage((uint32_t)corrected);

    // Instantaneous current — signed, shows DC offset directly
    int32_t instantIA = mySensor.getInstantaneousIA();

    // Convert to amps
    float amps = rawToAmps(averaged);

    // Output
    if (plotterMode)
    {
        // CSV format for Arduino Serial Plotter
        // Columns: RawIRMS, Corrected, Averaged, Amps*1000000, InstantIA
        Serial.print(rawIrms);
        Serial.print(",");
        Serial.print(corrected);
        Serial.print(",");
        Serial.print(averaged);
        Serial.print(",");
        Serial.print((int32_t)(amps * 1000000.0)); // microamps for plotter scale
        Serial.print(",");
        Serial.println(instantIA);
    }
    else
    {
        // Formatted table for Serial Monitor
        // Avoid snprintf %f — not supported on AVR
        char line[80];
        snprintf(line, sizeof(line),
                 "%9lu | %8lu | %9ld | %8lu | ",
                 (unsigned long)millis(),
                 (unsigned long)rawIrms,
                 (long)corrected,
                 (unsigned long)averaged);
        Serial.print(line);
        Serial.print(amps, 4);
        Serial.print(F("A | "));
        Serial.println(instantIA);
    }

    delay(SAMPLE_INTERVAL_MS);
}
