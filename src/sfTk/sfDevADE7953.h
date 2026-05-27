/**
 * @file sfDevADE7953.h
 * @brief Header file for the SparkFun ADE7953 Energy Meter Driver.
 *
 * @details
 * sfDevADE7953 is a comms-agnostic driver for the Analog Devices ADE7953 single-phase energy
 * metering IC, built on the SparkFun Toolkit. This driver focuses on current measurement via
 * CT clamp (Channel A primary, Channel B optional) with support for gain configuration,
 * zero-crossing detection, peak measurement, and interrupt handling.
 *
 * The ADE7953 uses 16-bit register addresses. The address range determines data width:
 *   0x0xx = 8-bit registers
 *   0x1xx = 16-bit registers
 *   0x2xx = 24-bit registers (lower 24 bits of 32-bit value)
 *   0x3xx = 32-bit registers (upper 8 bits are sign-extended)
 * This driver uses the 32-bit addresses (0x3xx) for all 24/32-bit registers.
 *
 * @author SparkFun Electronics
 * @date 2025
 * @copyright Copyright (c) 2025, SparkFun Electronics Inc. This project is released under the MIT License.
 *
 * SPDX-License-Identifier: MIT
 *
 * @see https://github.com/sparkfun/SparkFun_ADE7953_Arduino_Library
 */

#pragma once

#include <stdint.h>

// SparkFun Toolkit core headers
#include <sfTk/sfToolkit.h>
#include <sfTk/sfTkII2C.h>

///////////////////////////////////////////////////////////////////////////////
// I2C Addressing
///////////////////////////////////////////////////////////////////////////////
const uint8_t kADE7953DefaultAddr = 0x38; ///< Default 7-bit I2C address for the ADE7953.

///////////////////////////////////////////////////////////////////////////////
// 8-Bit Register Addresses (0x0xx)
///////////////////////////////////////////////////////////////////////////////
const uint16_t ksfADE7953RegSagCyc = 0x000;       ///< Sag line cycles
const uint16_t ksfADE7953RegDisNoLoad = 0x001;     ///< No-load detection disable
const uint16_t ksfADE7953RegLCycMode = 0x004;      ///< Line cycle accumulation mode config
const uint16_t ksfADE7953RegPgaV = 0x007;          ///< Voltage channel PGA gain (Bits[2:0])
const uint16_t ksfADE7953RegPgaIA = 0x008;         ///< Current Channel A PGA gain (Bits[2:0])
const uint16_t ksfADE7953RegPgaIB = 0x009;         ///< Current Channel B PGA gain (Bits[2:0])
const uint16_t ksfADE7953RegWriteProtect = 0x040;  ///< Write protection bits (Bits[2:0])
const uint16_t ksfADE7953RegLastOp = 0x0FD;        ///< Last operation type (0x35=read, 0xCA=write)
const uint16_t ksfADE7953RegLastRwData8 = 0x0FF;   ///< Last successful 8-bit register data
const uint16_t ksfADE7953RegVersion = 0x702;       ///< Silicon version number
const uint16_t ksfADE7953RegExRef = 0x800;         ///< Reference input config (0=internal, 1=external)

// Unlock register — write 0xAD here to unlock register 0x120
const uint16_t ksfADE7953RegUnlock = 0x0FE;        ///< Register unlock
const uint8_t ksfADE7953UnlockKey = 0xAD;          ///< Unlock key value

///////////////////////////////////////////////////////////////////////////////
// 16-Bit Register Addresses (0x1xx)
///////////////////////////////////////////////////////////////////////////////
const uint16_t ksfADE7953RegZXTout = 0x100;        ///< Zero-crossing timeout
const uint16_t ksfADE7953RegLineCyc = 0x101;       ///< Half line cycles for line cycle accumulation
const uint16_t ksfADE7953RegConfig = 0x102;        ///< Configuration register
const uint16_t ksfADE7953RegCF1Den = 0x103;        ///< CF1 frequency divider denominator
const uint16_t ksfADE7953RegCF2Den = 0x104;        ///< CF2 frequency divider denominator
const uint16_t ksfADE7953RegCFMode = 0x107;        ///< CF output selection
const uint16_t ksfADE7953RegPhCalA = 0x108;        ///< Phase calibration (Channel A, sign-magnitude)
const uint16_t ksfADE7953RegPhCalB = 0x109;        ///< Phase calibration (Channel B, sign-magnitude)
const uint16_t ksfADE7953RegPFA = 0x10A;           ///< Power factor (Channel A)
const uint16_t ksfADE7953RegPFB = 0x10B;           ///< Power factor (Channel B)
const uint16_t ksfADE7953RegAngleA = 0x10C;        ///< Angle between voltage and Current Channel A
const uint16_t ksfADE7953RegAngleB = 0x10D;        ///< Angle between voltage and Current Channel B
const uint16_t ksfADE7953RegPeriod = 0x10E;        ///< Period register (line period from ZX)
const uint16_t ksfADE7953RegAltOutput = 0x110;     ///< Alternative output functions
const uint16_t ksfADE7953RegOptimize = 0x120;      ///< Performance optimization (set to 0x0030)
const uint16_t ksfADE7953RegLastAdd = 0x1FE;       ///< Last successful communication address
const uint16_t ksfADE7953RegLastRwData16 = 0x1FF;  ///< Last successful 16-bit register data

// Optimization value required by datasheet for specified performance
const uint16_t ksfADE7953OptimizeValue = 0x0030;

///////////////////////////////////////////////////////////////////////////////
// 32-Bit Register Addresses (0x3xx)
// These registers contain valid data in the lower 24 bits; upper 8 bits are sign-extended.
///////////////////////////////////////////////////////////////////////////////

// --- Threshold and configuration ---
const uint16_t ksfADE7953RegSagLvl = 0x300;        ///< Sag voltage level
const uint16_t ksfADE7953RegAccMode = 0x301;       ///< Accumulation mode
const uint16_t ksfADE7953RegApNoLoad = 0x303;      ///< Active power no-load level
const uint16_t ksfADE7953RegVarNoLoad = 0x304;     ///< Reactive power no-load level
const uint16_t ksfADE7953RegVaNoLoad = 0x305;      ///< Apparent power no-load level

// --- Instantaneous values ---
const uint16_t ksfADE7953RegAVA = 0x310;           ///< Instantaneous apparent power (Channel A)
const uint16_t ksfADE7953RegBVA = 0x311;           ///< Instantaneous apparent power (Channel B)
const uint16_t ksfADE7953RegAWatt = 0x312;         ///< Instantaneous active power (Channel A)
const uint16_t ksfADE7953RegBWatt = 0x313;         ///< Instantaneous active power (Channel B)
const uint16_t ksfADE7953RegAVar = 0x314;          ///< Instantaneous reactive power (Channel A)
const uint16_t ksfADE7953RegBVar = 0x315;          ///< Instantaneous reactive power (Channel B)
const uint16_t ksfADE7953RegIA = 0x316;            ///< Instantaneous current (Channel A)
const uint16_t ksfADE7953RegIB = 0x317;            ///< Instantaneous current (Channel B)
const uint16_t ksfADE7953RegV = 0x318;             ///< Instantaneous voltage

// --- RMS values ---
const uint16_t ksfADE7953RegIRmsA = 0x31A;         ///< IRMS (Channel A)
const uint16_t ksfADE7953RegIRmsB = 0x31B;         ///< IRMS (Channel B)
const uint16_t ksfADE7953RegVRms = 0x31C;          ///< VRMS

// --- Energy accumulators ---
const uint16_t ksfADE7953RegAEnergyA = 0x31E;      ///< Active energy (Channel A)
const uint16_t ksfADE7953RegAEnergyB = 0x31F;      ///< Active energy (Channel B)
const uint16_t ksfADE7953RegREnergyA = 0x320;      ///< Reactive energy (Channel A)
const uint16_t ksfADE7953RegREnergyB = 0x321;      ///< Reactive energy (Channel B)
const uint16_t ksfADE7953RegApEnergyA = 0x322;     ///< Apparent energy (Channel A)
const uint16_t ksfADE7953RegApEnergyB = 0x323;     ///< Apparent energy (Channel B)

// --- Peak and overcurrent ---
const uint16_t ksfADE7953RegOvLvl = 0x324;         ///< Overvoltage level
const uint16_t ksfADE7953RegOiLvl = 0x325;         ///< Overcurrent level
const uint16_t ksfADE7953RegVPeak = 0x326;         ///< Voltage channel peak
const uint16_t ksfADE7953RegRstVPeak = 0x327;      ///< Voltage peak read with reset
const uint16_t ksfADE7953RegIAPeak = 0x328;        ///< Current Channel A peak
const uint16_t ksfADE7953RegRstIAPeak = 0x329;     ///< Current Channel A peak read with reset
const uint16_t ksfADE7953RegIBPeak = 0x32A;        ///< Current Channel B peak
const uint16_t ksfADE7953RegRstIBPeak = 0x32B;     ///< Current Channel B peak read with reset

// --- Interrupts ---
const uint16_t ksfADE7953RegIrqEnA = 0x32C;        ///< Interrupt enable (Channel A + voltage)
const uint16_t ksfADE7953RegIrqStatA = 0x32D;      ///< Interrupt status (Channel A + voltage)
const uint16_t ksfADE7953RegRstIrqStatA = 0x32E;   ///< Reset interrupt status (Channel A + voltage)
const uint16_t ksfADE7953RegIrqEnB = 0x32F;        ///< Interrupt enable (Channel B)
const uint16_t ksfADE7953RegIrqStatB = 0x330;      ///< Interrupt status (Channel B)
const uint16_t ksfADE7953RegRstIrqStatB = 0x331;   ///< Reset interrupt status (Channel B)

// --- CRC ---
const uint16_t ksfADE7953RegCRC = 0x37F;           ///< Checksum (32-bit only, no 24-bit access)

// --- Gain registers (Channel A) ---
const uint16_t ksfADE7953RegAIGain = 0x380;        ///< Current channel gain (Channel A)
const uint16_t ksfADE7953RegAVGain = 0x381;        ///< Voltage channel gain
const uint16_t ksfADE7953RegAWGain = 0x382;        ///< Active power gain (Channel A)
const uint16_t ksfADE7953RegAVarGain = 0x383;      ///< Reactive power gain (Channel A)
const uint16_t ksfADE7953RegAVaGain = 0x384;       ///< Apparent power gain (Channel A)
const uint16_t ksfADE7953RegAIRmsOS = 0x386;       ///< IRMS offset (Channel A)
const uint16_t ksfADE7953RegVRmsOS = 0x388;        ///< VRMS offset
const uint16_t ksfADE7953RegAWattOS = 0x389;       ///< Active power offset (Channel A)
const uint16_t ksfADE7953RegAVarOS = 0x38A;        ///< Reactive power offset (Channel A)
const uint16_t ksfADE7953RegAVaOS = 0x38B;         ///< Apparent power offset (Channel A)

// --- Gain registers (Channel B) ---
const uint16_t ksfADE7953RegBIGain = 0x38C;        ///< Current channel gain (Channel B)
const uint16_t ksfADE7953RegBWGain = 0x38E;        ///< Active power gain (Channel B)
const uint16_t ksfADE7953RegBVarGain = 0x38F;      ///< Reactive power gain (Channel B)
const uint16_t ksfADE7953RegBVaGain = 0x390;       ///< Apparent power gain (Channel B)
const uint16_t ksfADE7953RegBIRmsOS = 0x392;       ///< IRMS offset (Channel B)
const uint16_t ksfADE7953RegBWattOS = 0x395;       ///< Active power offset (Channel B)
const uint16_t ksfADE7953RegBVarOS = 0x396;        ///< Reactive power offset (Channel B)
const uint16_t ksfADE7953RegBVaOS = 0x397;         ///< Apparent power offset (Channel B)

// --- Last data (32-bit) ---
const uint16_t ksfADE7953RegLastRwData32 = 0x3FF;  ///< Last successful 24/32-bit register data

///////////////////////////////////////////////////////////////////////////////
// PGA Gain Values
///////////////////////////////////////////////////////////////////////////////
/// @brief PGA gain settings for PGA_V, PGA_IA, PGA_IB registers.
/// These map to Bits[2:0] of the respective register.
enum sfe_ade7953_pga_gain_t : uint8_t
{
    ADE7953_PGA_GAIN_1 = 0,   ///< 1x gain
    ADE7953_PGA_GAIN_2 = 1,   ///< 2x gain
    ADE7953_PGA_GAIN_4 = 2,   ///< 4x gain
    ADE7953_PGA_GAIN_8 = 3,   ///< 8x gain
    ADE7953_PGA_GAIN_16 = 4,  ///< 16x gain
    ADE7953_PGA_GAIN_22 = 5   ///< 22x gain (current channels only)
};

///////////////////////////////////////////////////////////////////////////////
// Zero-Crossing Edge Selection
///////////////////////////////////////////////////////////////////////////////
/// @brief Zero-crossing edge selection for CONFIG register bits [13:12].
enum sfe_ade7953_zx_edge_t : uint8_t
{
    ADE7953_ZX_EDGE_BOTH = 0,      ///< Interrupt on both positive and negative-going ZX
    ADE7953_ZX_EDGE_NEGATIVE = 1,  ///< Interrupt on negative-going ZX only
    ADE7953_ZX_EDGE_POSITIVE = 2,  ///< Interrupt on positive-going ZX only
    ADE7953_ZX_EDGE_BOTH_ALT = 3   ///< Same as BOTH (alternate encoding)
};

///////////////////////////////////////////////////////////////////////////////
// CONFIG Register Bit Definitions (Address 0x102)
///////////////////////////////////////////////////////////////////////////////
const uint16_t ksfADE7953ConfigIntEnA = (1 << 0);    ///< Integrator enable (Channel A)
const uint16_t ksfADE7953ConfigIntEnB = (1 << 1);    ///< Integrator enable (Channel B)
const uint16_t ksfADE7953ConfigHPFEn = (1 << 2);     ///< HPF enable (all channels)
const uint16_t ksfADE7953ConfigPFMode = (1 << 3);    ///< Power factor mode
const uint16_t ksfADE7953ConfigRevpCF = (1 << 4);    ///< REVP updated on CF1(0) or CF2(1)
const uint16_t ksfADE7953ConfigRevpPulse = (1 << 5); ///< REVP output mode
const uint16_t ksfADE7953ConfigZXLPF = (1 << 6);     ///< ZX LPF disable (1=disabled)
const uint16_t ksfADE7953ConfigSWRst = (1 << 7);     ///< Software reset
const uint16_t ksfADE7953ConfigCRCEn = (1 << 8);     ///< CRC enable
const uint16_t ksfADE7953ConfigZXI = (1 << 11);      ///< ZX_I source: 0=Channel A, 1=Channel B
const uint16_t ksfADE7953ConfigCommLock = (1 << 15);  ///< Communication locking (1=disabled)

// CONFIG register ZX_EDGE field mask and shift
const uint16_t ksfADE7953ConfigZXEdgeMask = 0x3000;  ///< Bits [13:12]
const uint8_t ksfADE7953ConfigZXEdgeShift = 12;

///////////////////////////////////////////////////////////////////////////////
// LCYCMODE Register Bit Definitions (Address 0x004)
///////////////////////////////////////////////////////////////////////////////
const uint8_t ksfADE7953LCycALWatt = (1 << 0);  ///< Active energy line cycle mode (Channel A)
const uint8_t ksfADE7953LCycBLWatt = (1 << 1);  ///< Active energy line cycle mode (Channel B)
const uint8_t ksfADE7953LCycALVar = (1 << 2);   ///< Reactive energy line cycle mode (Channel A)
const uint8_t ksfADE7953LCycBLVar = (1 << 3);   ///< Reactive energy line cycle mode (Channel B)
const uint8_t ksfADE7953LCycALVa = (1 << 4);    ///< Apparent energy line cycle mode (Channel A)
const uint8_t ksfADE7953LCycBLVa = (1 << 5);    ///< Apparent energy line cycle mode (Channel B)
const uint8_t ksfADE7953LCycRstRead = (1 << 6); ///< Read-with-reset enable for all registers

///////////////////////////////////////////////////////////////////////////////
// DISNOLOAD Register Bit Definitions (Address 0x001)
///////////////////////////////////////////////////////////////////////////////
const uint8_t ksfADE7953NoLoadDisAP = (1 << 0);  ///< Disable active power no-load detection
const uint8_t ksfADE7953NoLoadDisVAR = (1 << 1); ///< Disable reactive power no-load detection
const uint8_t ksfADE7953NoLoadDisVA = (1 << 2);  ///< Disable apparent power no-load detection

///////////////////////////////////////////////////////////////////////////////
// Interrupt Bitmask Constants
// Used with IRQENA/IRQSTATA (Channel A) and IRQENB/IRQSTATB (Channel B).
// Channel B interrupts use bits [13:0] only.
///////////////////////////////////////////////////////////////////////////////
const uint32_t ksfADE7953IrqAEHF = (1UL << 0);       ///< Active energy half full
const uint32_t ksfADE7953IrqVAREHF = (1UL << 1);     ///< Reactive energy half full
const uint32_t ksfADE7953IrqVAEHF = (1UL << 2);      ///< Apparent energy half full
const uint32_t ksfADE7953IrqAEOF = (1UL << 3);       ///< Active energy overflow/underflow
const uint32_t ksfADE7953IrqVAREOF = (1UL << 4);     ///< Reactive energy overflow/underflow
const uint32_t ksfADE7953IrqVAEOF = (1UL << 5);      ///< Apparent energy overflow/underflow
const uint32_t ksfADE7953IrqApNoLoad = (1UL << 6);    ///< Active power no-load detected
const uint32_t ksfADE7953IrqVarNoLoad = (1UL << 7);   ///< Reactive power no-load detected
const uint32_t ksfADE7953IrqVaNoLoad = (1UL << 8);    ///< Apparent power no-load detected
const uint32_t ksfADE7953IrqApSign = (1UL << 9);      ///< Active energy sign changed
const uint32_t ksfADE7953IrqVarSign = (1UL << 10);    ///< Reactive energy sign changed
const uint32_t ksfADE7953IrqZXToI = (1UL << 11);      ///< Current zero-crossing timeout
const uint32_t ksfADE7953IrqZXI = (1UL << 12);        ///< Current zero-crossing detected
const uint32_t ksfADE7953IrqOI = (1UL << 13);         ///< Overcurrent threshold exceeded
// Bits 14-21 are Channel A / voltage only (not present in IRQENB/IRQSTATB)
const uint32_t ksfADE7953IrqZXToV = (1UL << 14);      ///< Voltage zero-crossing timeout
const uint32_t ksfADE7953IrqZXV = (1UL << 15);        ///< Voltage zero-crossing detected
const uint32_t ksfADE7953IrqOV = (1UL << 16);         ///< Overvoltage threshold exceeded
const uint32_t ksfADE7953IrqWSmp = (1UL << 17);       ///< New waveform data acquired
const uint32_t ksfADE7953IrqCycEnd = (1UL << 18);     ///< End of line cycle accumulation period
const uint32_t ksfADE7953IrqSag = (1UL << 19);        ///< Sag event occurred
const uint32_t ksfADE7953IrqReset = (1UL << 20);      ///< Reset occurred (always enabled)
const uint32_t ksfADE7953IrqCRC = (1UL << 21);        ///< Checksum changed

///////////////////////////////////////////////////////////////////////////////
// LAST_OP Values
///////////////////////////////////////////////////////////////////////////////
const uint8_t ksfADE7953LastOpRead = 0x35;  ///< Last operation was a read
const uint8_t ksfADE7953LastOpWrite = 0xCA; ///< Last operation was a write

///////////////////////////////////////////////////////////////////////////////
// Class Declaration
///////////////////////////////////////////////////////////////////////////////

/// @brief Platform-independent driver for the ADE7953 energy metering IC.
///
/// @details This class implements register-level access to the ADE7953 via the SparkFun Toolkit
/// bus interface. It is focused on current measurement (CT clamp on Channel A, optional Channel B)
/// with support for gain configuration, zero-crossing detection, peak measurement, calibration
/// offsets, and interrupt management.
///
/// This class does not depend on Arduino — the Arduino-specific wrapper (SfeADE7953ArdI2C) provides
/// the begin() and isConnected() methods that initialize the Toolkit I2C bus.
class sfDevADE7953
{
  public:
    sfDevADE7953() : _theBus{nullptr}
    {
    }

    /// @brief Initialize the device driver with the given bus.
    /// @details Sets the communication bus, configures byte order for big-endian,
    /// and writes the optimized performance register (0x120 = 0x0030) as required by the datasheet.
    /// @param theBus Pointer to the initialized bus object.
    /// @return ksfTkErrOk on success, or an error code on failure.
    bool begin(sfTkIBus *theBus = nullptr);

    /// @brief Set the communication bus pointer.
    /// @param theBus Bus to use for all register I/O.
    void setCommunicationBus(sfTkIBus *theBus);

    // ========================= Identity & Setup ===========================

    /// @brief Read the silicon version number from the Version register (0x702).
    /// @return Silicon version, or 0 on error.
    uint8_t getVersion(void);

    /// @brief Perform a software reset via the CONFIG register SWRST bit.
    /// @return True on success, false on error.
    bool reset(void);

    /// @brief Set write protection bits.
    /// @param protect Write protection value (Bits[2:0]).
    /// @return True on success, false on error.
    bool setWriteProtect(uint8_t protect);

    /// @brief Get the current write protection setting.
    /// @return Write protection value, or 0 on error.
    uint8_t getWriteProtect(void);

    // ======================== PGA Gain Configuration ======================

    /// @brief Set the PGA gain for Current Channel A.
    /// @param gain PGA gain setting (see sfe_ade7953_pga_gain_t).
    /// @return True on success, false on error.
    bool setGainIA(sfe_ade7953_pga_gain_t gain);

    /// @brief Get the PGA gain for Current Channel A.
    /// @return PGA gain setting, or ADE7953_PGA_GAIN_1 on error.
    sfe_ade7953_pga_gain_t getGainIA(void);

    /// @brief Set the PGA gain for Current Channel B.
    /// @param gain PGA gain setting.
    /// @return True on success, false on error.
    bool setGainIB(sfe_ade7953_pga_gain_t gain);

    /// @brief Get the PGA gain for Current Channel B.
    /// @return PGA gain setting, or ADE7953_PGA_GAIN_1 on error.
    sfe_ade7953_pga_gain_t getGainIB(void);

    /// @brief Set the PGA gain for the voltage channel.
    /// @note Even in current-only configurations, this affects ZX detection behavior.
    /// @param gain PGA gain setting (ADE7953_PGA_GAIN_22 not valid for voltage).
    /// @return True on success, false on error.
    bool setGainV(sfe_ade7953_pga_gain_t gain);

    /// @brief Get the PGA gain for the voltage channel.
    /// @return PGA gain setting, or ADE7953_PGA_GAIN_1 on error.
    sfe_ade7953_pga_gain_t getGainV(void);

    /// @brief Set the digital gain for Current Channel A.
    /// @param gain Digital gain value (default 0x400000).
    /// @return True on success, false on error.
    bool setDigitalGainIA(uint32_t gain);

    /// @brief Get the digital gain for Current Channel A.
    /// @return Digital gain value, or 0 on error.
    uint32_t getDigitalGainIA(void);

    /// @brief Set the digital gain for Current Channel B.
    /// @param gain Digital gain value (default 0x400000).
    /// @return True on success, false on error.
    bool setDigitalGainIB(uint32_t gain);

    /// @brief Get the digital gain for Current Channel B.
    /// @return Digital gain value, or 0 on error.
    uint32_t getDigitalGainIB(void);

    // ======================= Current Measurement ==========================

    /// @brief Read the IRMS value for Current Channel A.
    /// @return Raw unsigned IRMS value, or 0 on error.
    uint32_t getIRmsA(void);

    /// @brief Read the IRMS value for Current Channel B.
    /// @return Raw unsigned IRMS value, or 0 on error.
    uint32_t getIRmsB(void);

    /// @brief Read the instantaneous current sample for Channel A.
    /// @return Signed instantaneous current value, or 0 on error.
    int32_t getInstantaneousIA(void);

    /// @brief Read the instantaneous current sample for Channel B.
    /// @return Signed instantaneous current value, or 0 on error.
    int32_t getInstantaneousIB(void);

    // ======================== Peak Detection ==============================

    /// @brief Read the peak current value for Channel A.
    /// @return Peak value, or 0 on error.
    uint32_t getPeakIA(void);

    /// @brief Read the peak current value for Channel B.
    /// @return Peak value, or 0 on error.
    uint32_t getPeakIB(void);

    /// @brief Read and reset the peak current value for Channel A.
    /// @details Atomically reads the peak and clears the register.
    /// @return Peak value before reset, or 0 on error.
    uint32_t readAndResetPeakIA(void);

    /// @brief Read and reset the peak current value for Channel B.
    /// @details Atomically reads the peak and clears the register.
    /// @return Peak value before reset, or 0 on error.
    uint32_t readAndResetPeakIB(void);

    /// @brief Set the overcurrent detection threshold.
    /// @param level Threshold level (24-bit value).
    /// @return True on success, false on error.
    bool setOvercurrentLevel(uint32_t level);

    /// @brief Get the overcurrent detection threshold.
    /// @return Threshold level, or 0 on error.
    uint32_t getOvercurrentLevel(void);

    // ====================== Calibration & Offset ==========================

    /// @brief Set the IRMS offset for Current Channel A.
    /// @param offset Signed offset value for IRMS correction.
    /// @return True on success, false on error.
    bool setIRmsOffsetA(int32_t offset);

    /// @brief Get the IRMS offset for Current Channel A.
    /// @return Signed offset value, or 0 on error.
    int32_t getIRmsOffsetA(void);

    /// @brief Set the IRMS offset for Current Channel B.
    /// @param offset Signed offset value for IRMS correction.
    /// @return True on success, false on error.
    bool setIRmsOffsetB(int32_t offset);

    /// @brief Get the IRMS offset for Current Channel B.
    /// @return Signed offset value, or 0 on error.
    int32_t getIRmsOffsetB(void);

    // ======================= Zero-Crossing (ZX_I) =========================

    /// @brief Select which current channel drives the ZX_I output.
    /// @param useChannelB If true, ZX_I is based on Channel B; if false, Channel A.
    /// @return True on success, false on error.
    bool setZXISource(bool useChannelB);

    /// @brief Get which current channel drives the ZX_I output.
    /// @return True if Channel B, false if Channel A (or on error).
    bool getZXISource(void);

    /// @brief Set the zero-crossing edge selection for interrupt generation.
    /// @param edge Edge selection (see sfe_ade7953_zx_edge_t).
    /// @return True on success, false on error.
    bool setZXEdge(sfe_ade7953_zx_edge_t edge);

    /// @brief Get the current zero-crossing edge selection.
    /// @return Edge selection value.
    sfe_ade7953_zx_edge_t getZXEdge(void);

    /// @brief Enable or disable the zero-crossing low-pass filter.
    /// @param enable True to enable the LPF, false to disable.
    /// @return True on success, false on error.
    bool enableZXLPF(bool enable);

    /// @brief Enable or disable the high-pass filter on all channels.
    /// @details The HPF removes DC offset from the current and voltage channels before
    /// RMS computation. Should be enabled for accurate IRMS readings. Enabled by default
    /// after power-on reset.
    /// @param enable True to enable the HPF, false to disable.
    /// @return True on success, false on error.
    bool enableHPF(bool enable);

    /// @brief Check whether the high-pass filter is currently enabled.
    /// @return True if HPF is enabled, false if disabled or on error.
    bool isHPFEnabled(void);

    /// @brief Read the line period derived from zero-crossing detection.
    /// @return Period register value, or 0 on error.
    uint16_t getPeriod(void);

    // =========================== Interrupts ===============================

    /// @brief Set the interrupt enable mask for Channel A (and voltage).
    /// @details Use the ksfADE7953Irq* bitmask constants to build the mask.
    /// @param mask Interrupt enable bitmask.
    /// @return True on success, false on error.
    bool setInterruptEnableA(uint32_t mask);

    /// @brief Get the interrupt enable mask for Channel A.
    /// @return Interrupt enable bitmask, or 0 on error.
    uint32_t getInterruptEnableA(void);

    /// @brief Read the interrupt status for Channel A (non-destructive).
    /// @return Interrupt status bitmask, or 0 on error.
    uint32_t getInterruptStatusA(void);

    /// @brief Read and reset the interrupt status for Channel A.
    /// @details This atomically reads the status and clears the flags.
    /// @return Interrupt status bitmask before reset, or 0 on error.
    uint32_t readAndResetInterruptStatusA(void);

    /// @brief Set the interrupt enable mask for Channel B.
    /// @details Only bits [13:0] are valid for Channel B.
    /// @param mask Interrupt enable bitmask.
    /// @return True on success, false on error.
    bool setInterruptEnableB(uint32_t mask);

    /// @brief Get the interrupt enable mask for Channel B.
    /// @return Interrupt enable bitmask, or 0 on error.
    uint32_t getInterruptEnableB(void);

    /// @brief Read the interrupt status for Channel B (non-destructive).
    /// @return Interrupt status bitmask, or 0 on error.
    uint32_t getInterruptStatusB(void);

    /// @brief Read and reset the interrupt status for Channel B.
    /// @return Interrupt status bitmask before reset, or 0 on error.
    uint32_t readAndResetInterruptStatusB(void);

    // ========================= No-Load Detection ==========================

    /// @brief Set the no-load detection disable register.
    /// @param mask Bitmask of no-load features to disable (see ksfADE7953NoLoadDis* constants).
    /// @return True on success, false on error.
    bool setNoLoadDisable(uint8_t mask);

    /// @brief Get the no-load detection disable register.
    /// @return Current disable bitmask, or 0 on error.
    uint8_t getNoLoadDisable(void);

    // ====================== Line Cycle Accumulation =======================

    /// @brief Set the line cycle accumulation mode register.
    /// @param mode Bitmask of line cycle modes (see ksfADE7953LCyc* constants).
    /// @return True on success, false on error.
    bool setLineCycleMode(uint8_t mode);

    /// @brief Get the line cycle accumulation mode register.
    /// @return Mode bitmask, or 0 on error.
    uint8_t getLineCycleMode(void);

    /// @brief Set the number of half line cycles for line cycle accumulation.
    /// @param halfCycles Number of half cycles.
    /// @return True on success, false on error.
    bool setLineCycleCount(uint16_t halfCycles);

    /// @brief Get the number of half line cycles for line cycle accumulation.
    /// @return Number of half cycles, or 0 on error.
    uint16_t getLineCycleCount(void);

    // ========================= Sag Detection ==============================

    /// @brief Set the number of sag line cycles.
    /// @param cycles Number of half line cycles for sag detection.
    /// @return True on success, false on error.
    bool setSagCycles(uint8_t cycles);

    /// @brief Get the number of sag line cycles.
    /// @return Number of half line cycles, or 0 on error.
    uint8_t getSagCycles(void);

    /// @brief Set the sag voltage level threshold.
    /// @param level Sag level threshold (24-bit value).
    /// @return True on success, false on error.
    bool setSagLevel(uint32_t level);

    /// @brief Get the sag voltage level threshold.
    /// @return Sag level, or 0 on error.
    uint32_t getSagLevel(void);

    // ========================== Diagnostics ===============================

    /// @brief Get the type of the last successful communication.
    /// @return 0x35 for read, 0xCA for write, or 0 on error.
    uint8_t getLastOperation(void);

    /// @brief Get the address of the last successful communication.
    /// @return Register address, or 0 on error.
    uint16_t getLastAddress(void);

    /// @brief Get the data from the last successful 8-bit register communication.
    /// @return Data value, or 0 on error.
    uint8_t getLastData8(void);

    /// @brief Get the data from the last successful 16-bit register communication.
    /// @return Data value, or 0 on error.
    uint16_t getLastData16(void);

    /// @brief Get the data from the last successful 24/32-bit register communication.
    /// @return Data value, or 0 on error.
    uint32_t getLastData32(void);

  protected:
    sfTkIBus *_theBus; ///< Pointer to the communication bus device.
};