/*
  Example 01 - Basic Current Reading

  Output is formatted for the Arduino Serial Plotter (labeled values):
    Raw I:XXXXX;Filtered I:XXXXX;Averaged:XXXXX;Amps:X.XXXX

  The PGA gain is set to 4x for the 5.6 ohm shunt resistor on the
  SparkFun Qwiic Current Sensor board.

  Zero calibration runs automatically at startup — make sure NO current
  is flowing through the CT clamp when powering on.

  SparkFun Electronics
  Date: 2025
  SparkFun code, firmware, and software is released under the MIT License.
    Please see LICENSE.md for further details.

  Hardware Connections:
  IoT RedBoard --> ADE7953
  QWIIC --> QWIIC

  Serial.print it out at 115200 baud to serial monitor / serial plotter.

  Feel like supporting our work? Buy a board from SparkFun!
  https://www.sparkfun.com/sparkfun-qwiic-current-sensor.html
*/

#include <SparkFun_ADE7953.h>

SfeADE7953ArdI2C mySensor;

// ========================= Configuration ==========================
// Modify these for your CT / burden resistor setup.
const float CT_RATIO        = 2000.0;    // CT turns ratio (30A:15mA = 2000:1)
const float BURDEN_RESISTOR = 5.6;       // Burden resistor in ohms
const float FULL_SCALE_CODE = 5928256.0; // ADE7953 full-scale IRMS code
const float FULL_SCALE_VRMS = 0.35355;   // 0.5V peak / sqrt(2)

const int AVERAGE_WINDOW    = 20;        // Rolling average sample count
const int SAMPLE_INTERVAL_MS = 100;      // Milliseconds between reads
const int ZERO_CAL_SAMPLES  = 50;        // Samples to average for zero cal

// ========================= State ==================================
uint32_t irmsBuffer[AVERAGE_WINDOW];
int      bufferIndex  = 0;
bool     bufferFull   = false;
int32_t  zeroOffset   = 0;        // Software zero offset (raw IRMS units)
float    pgaMultiplier = 4.0;     // Tracks current PGA for amps conversion

// ========================= Helpers ================================

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
        bufferFull  = true;
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
    bufferFull  = false;
    memset(irmsBuffer, 0, sizeof(irmsBuffer));
}

// ========================= Auto Zero Calibration ==================

/// @brief Collect baseline samples with no current flowing and store as offset.
///        Called once during setup — keep CT clamped with no load attached.
void autoZeroCalibration()
{
    Serial.println("Auto zero calibration — ensure NO current is flowing.");
    Serial.print("Collecting ");
    Serial.print(ZERO_CAL_SAMPLES);
    Serial.println(" samples...");

    uint64_t sum = 0;
    for (int i = 0; i < ZERO_CAL_SAMPLES; i++)
    {
        sum += mySensor.getIRmsA();
        delay(50);
    }

    zeroOffset = (int32_t)(sum / ZERO_CAL_SAMPLES);
    resetBuffer();

    Serial.print("Zero offset set to: ");
    Serial.println(zeroOffset);
    Serial.println();
}

// ========================= Setup / Loop ===========================

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("ADE7953 Example 01 Enhanced - Basic Current Reading");

    Wire.begin();

    if (!mySensor.begin())
    {
        Serial.println("ADE7953 not found. Please check wiring. Freezing...");
        while (1)
            delay(1000);
    }

    Serial.print("ADE7953 connected!");

    // Set PGA gain to 4x for the 5.6 ohm shunt resistor on the board.
    if (!mySensor.setGainIA(ADE7953_PGA_GAIN_16))
    {
        Serial.println("Failed to set gain. Freezing...");
        while (1)
            delay(1000);
    }

    // Enable HPF to remove DC offset from the current channel.
    mySensor.enableHPF(true);

    Serial.println(" Gain: 16x | HPF: enabled");

    // Let the DSP settle before sampling.
    Serial.println("Waiting for DSP to settle...");
    delay(1000);

    memset(irmsBuffer, 0, sizeof(irmsBuffer));

    // Run automatic zero calibration — no current should be flowing yet.
    autoZeroCalibration();

    Serial.println("Reading current...");
    Serial.println();
}

void loop()
{
    // 1. Read raw IRMS from Channel A.
    uint32_t rawIrms = mySensor.getIRmsA();

    // 2. Subtract zero offset (clamped to 0).
    int32_t filtered = (int32_t)rawIrms - zeroOffset;
    if (filtered < 0)
        filtered = 0;

    // 3. Rolling average of the filtered value.
    uint32_t averaged = updateAverage((uint32_t)filtered);

    // 4. Convert averaged value to real amps.
    float amps = rawToAmps(averaged);

    // 5. Output — labeled format for Arduino Serial Plotter.
    //    Each label:value pair is separated by ; so the plotter
    //    treats them as separate named channels.
    Serial.print("RawI:");
    Serial.print(rawIrms);
    Serial.print(",");
    Serial.print("FilteredI:");
    Serial.print(filtered);
    Serial.print(",");
    Serial.print("Averaged:");
    Serial.print(averaged);
    Serial.print(",");
    Serial.print("Amps:");
    Serial.println(amps);

    delay(SAMPLE_INTERVAL_MS);
}
