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
// PGA Gain Values
///////////////////////////////////////////////////////////////////////////////
/// @brief PGA gain settings for the PGA_V, PGA_IA, and PGA_IB registers.
/// @details These map to Bits[2:0] of the respective register. A typedef is provided so the
/// type can be used without the @c enum keyword in environments where that matters.
typedef enum sfe_ade7953_pga_gain_t : uint8_t
{
    ADE7953_PGA_GAIN_1 = 0,  ///< 1x gain
    ADE7953_PGA_GAIN_2 = 1,  ///< 2x gain
    ADE7953_PGA_GAIN_4 = 2,  ///< 4x gain
    ADE7953_PGA_GAIN_8 = 3,  ///< 8x gain
    ADE7953_PGA_GAIN_16 = 4, ///< 16x gain
    ADE7953_PGA_GAIN_22 = 5  ///< 22x gain (current channels only; not valid for the voltage channel)
} sfe_ade7953_pga_gain_t;

///////////////////////////////////////////////////////////////////////////////
// Zero-Crossing Edge Selection
///////////////////////////////////////////////////////////////////////////////
/// @brief Zero-crossing edge selection for CONFIG register bits [13:12].
typedef enum sfe_ade7953_zx_edge_t : uint8_t
{
    ADE7953_ZX_EDGE_BOTH = 0,     ///< Interrupt on both positive and negative-going ZX
    ADE7953_ZX_EDGE_NEGATIVE = 1, ///< Interrupt on negative-going ZX only
    ADE7953_ZX_EDGE_POSITIVE = 2, ///< Interrupt on positive-going ZX only
    ADE7953_ZX_EDGE_BOTH_ALT = 3  ///< Same as BOTH (alternate encoding)
} sfe_ade7953_zx_edge_t;

///////////////////////////////////////////////////////////////////////////////
// CONFIG Register Bitfield (Address 0x102)
///////////////////////////////////////////////////////////////////////////////
/// @brief Bitfield union for the 16-bit CONFIG register (0x102).
union sfe_ade7953_config_reg_t
{
    struct
    {
        uint16_t intEnA : 1;    ///< Integrator enable (Channel A) [bit 0]
        uint16_t intEnB : 1;    ///< Integrator enable (Channel B) [bit 1]
        uint16_t hpfEn : 1;     ///< HPF enable (all channels) [bit 2]
        uint16_t pfMode : 1;    ///< Power factor mode [bit 3]
        uint16_t revpCF : 1;    ///< REVP updated on CF1(0) or CF2(1) [bit 4]
        uint16_t revpPulse : 1; ///< REVP output mode [bit 5]
        uint16_t zxLPF : 1;     ///< ZX LPF disable (1=disabled) [bit 6]
        uint16_t swRst : 1;     ///< Software reset [bit 7]
        uint16_t crcEn : 1;     ///< CRC enable [bit 8]
        uint16_t : 2;           ///< Reserved [bits 9-10]
        uint16_t zxI : 1;       ///< ZX_I source: 0=Channel A, 1=Channel B [bit 11]
        uint16_t zxEdge : 2;    ///< ZX edge selection (see sfe_ade7953_zx_edge_t) [bits 13:12]
        uint16_t : 1;           ///< Reserved [bit 14]
        uint16_t commLock : 1;  ///< Communication locking (1=disabled) [bit 15]
    };
    uint16_t word; ///< Raw register value for I2C read/write.
};

///////////////////////////////////////////////////////////////////////////////
// LCYCMODE Register Bitfield (Address 0x004)
///////////////////////////////////////////////////////////////////////////////
/// @brief Bitfield union for the 8-bit LCYCMODE register (0x004).
union sfe_ade7953_lcycmode_reg_t
{
    struct
    {
        uint8_t aLWatt : 1;  ///< Active energy line cycle mode (Channel A) [bit 0]
        uint8_t bLWatt : 1;  ///< Active energy line cycle mode (Channel B) [bit 1]
        uint8_t aLVar : 1;   ///< Reactive energy line cycle mode (Channel A) [bit 2]
        uint8_t bLVar : 1;   ///< Reactive energy line cycle mode (Channel B) [bit 3]
        uint8_t aLVa : 1;    ///< Apparent energy line cycle mode (Channel A) [bit 4]
        uint8_t bLVa : 1;    ///< Apparent energy line cycle mode (Channel B) [bit 5]
        uint8_t rstRead : 1; ///< Read-with-reset enable for all registers [bit 6]
        uint8_t : 1;         ///< Reserved [bit 7]
    };
    uint8_t byte; ///< Raw register value for I2C read/write.
};

///////////////////////////////////////////////////////////////////////////////
// DISNOLOAD Register Bitfield (Address 0x001)
///////////////////////////////////////////////////////////////////////////////
/// @brief Bitfield union for the 8-bit DISNOLOAD register (0x001).
union sfe_ade7953_disnoload_reg_t
{
    struct
    {
        uint8_t disAP : 1;  ///< Disable active power no-load detection [bit 0]
        uint8_t disVAR : 1; ///< Disable reactive power no-load detection [bit 1]
        uint8_t disVA : 1;  ///< Disable apparent power no-load detection [bit 2]
        uint8_t : 5;        ///< Reserved [bits 3-7]
    };
    uint8_t byte; ///< Raw register value for I2C read/write.
};

///////////////////////////////////////////////////////////////////////////////
// IRQ Register Bitfield
// Used with IRQENA/IRQSTATA (Channel A) and IRQENB/IRQSTATB (Channel B).
// Channel B interrupts use bits [13:0] only — bits [21:14] are Channel A / voltage only.
///////////////////////////////////////////////////////////////////////////////
/// @brief Bitfield union for the 32-bit IRQ enable and status registers.
union sfe_ade7953_irq_reg_t
{
    struct
    {
        uint32_t aehf : 1;      ///< Active energy half full [bit 0]
        uint32_t varehf : 1;    ///< Reactive energy half full [bit 1]
        uint32_t vaehf : 1;     ///< Apparent energy half full [bit 2]
        uint32_t aeof : 1;      ///< Active energy overflow/underflow [bit 3]
        uint32_t vareof : 1;    ///< Reactive energy overflow/underflow [bit 4]
        uint32_t vaeof : 1;     ///< Apparent energy overflow/underflow [bit 5]
        uint32_t apNoLoad : 1;  ///< Active power no-load detected [bit 6]
        uint32_t varNoLoad : 1; ///< Reactive power no-load detected [bit 7]
        uint32_t vaNoLoad : 1;  ///< Apparent power no-load detected [bit 8]
        uint32_t apSign : 1;    ///< Active energy sign changed [bit 9]
        uint32_t varSign : 1;   ///< Reactive energy sign changed [bit 10]
        uint32_t zxToI : 1;     ///< Current zero-crossing timeout [bit 11]
        uint32_t zxI : 1;       ///< Current zero-crossing detected [bit 12]
        uint32_t oI : 1;        ///< Overcurrent threshold exceeded [bit 13]
        uint32_t zxToV : 1;     ///< Voltage zero-crossing timeout (Channel A only) [bit 14]
        uint32_t zxV : 1;       ///< Voltage zero-crossing detected (Channel A only) [bit 15]
        uint32_t oV : 1;        ///< Overvoltage threshold exceeded (Channel A only) [bit 16]
        uint32_t wSmp : 1;      ///< New waveform data acquired (Channel A only) [bit 17]
        uint32_t cycEnd : 1;    ///< End of line cycle accumulation period (Channel A only) [bit 18]
        uint32_t sag : 1;       ///< Sag event occurred (Channel A only) [bit 19]
        uint32_t reset : 1;     ///< Reset occurred — always enabled (Channel A only) [bit 20]
        uint32_t crc : 1;       ///< Checksum changed (Channel A only) [bit 21]
        uint32_t : 10;          ///< Reserved [bits 22-31]
    };
    uint32_t word; ///< Raw register value for I2C read/write.
};

///////////////////////////////////////////////////////////////////////////////
// Class Declaration
///////////////////////////////////////////////////////////////////////////////

/// @brief Platform-independent driver for the ADE7953 energy metering IC.
///
/// @details This class implements register-level access to the ADE7953 via the SparkFun Toolkit
/// bus interface. Most methods return a SparkFun Toolkit error code (::ksfTkErrOk on success, a
/// negative value on error); values read from the device are returned through reference (output)
/// parameters. Callers that do not care about error handling can simply ignore the returned code.
class sfDevADE7953
{
  public:
    sfDevADE7953() : _theBus{nullptr}
    {
    }

    /// @brief Initialize the device driver with the given bus.
    /// @details Verifies the device is responding, configures big-endian byte order, applies the
    /// optimized performance register sequence required by the datasheet, and sets sensible default
    /// gain / HPF configuration so a basic sketch works without any further setup.
    /// @param theBus Pointer to the initialized bus object. If null, a bus set by a prior call is used.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t begin(sfTkIBus *theBus = nullptr);

    /// @brief Check whether the ADE7953 is connected and responding.
    /// @return true if the device responds, false otherwise.
    bool isConnected(void);

    // ========================= Identity & Setup ===========================

    /// @brief Read the silicon version number from the Version register (0x702).
    /// @param version Output reference that receives the silicon version.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getVersion(uint8_t &version);

    /// @brief Perform a software reset and re-apply the datasheet performance configuration.
    /// @details Asserts the CONFIG SWRST bit. Because a reset returns all registers to their
    /// power-on defaults, this method re-runs the unlock + optimize sequence (and default gain / HPF
    /// configuration) afterward so the device is left in the same state as a fresh begin().
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t reset(void);

    /// @brief Set write protection bits.
    /// @param protect Write protection value (Bits[2:0]).
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t setWriteProtect(uint8_t protect);

    /// @brief Get the current write protection setting.
    /// @param protect Output reference that receives the write protection value (Bits[2:0]).
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getWriteProtect(uint8_t &protect);

    // ======================== PGA Gain Configuration ======================

    /// @brief Set the PGA gain for Current Channel A.
    /// @param gain PGA gain setting (see sfe_ade7953_pga_gain_t).
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t setGainIA(sfe_ade7953_pga_gain_t gain);

    /// @brief Get the PGA gain for Current Channel A.
    /// @param gain Output reference that receives the PGA gain setting.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getGainIA(sfe_ade7953_pga_gain_t &gain);

    /// @brief Set the PGA gain for Current Channel B.
    /// @param gain PGA gain setting.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t setGainIB(sfe_ade7953_pga_gain_t gain);

    /// @brief Get the PGA gain for Current Channel B.
    /// @param gain Output reference that receives the PGA gain setting.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getGainIB(sfe_ade7953_pga_gain_t &gain);

    /// @brief Set the PGA gain for the voltage channel.
    /// @note ADE7953_PGA_GAIN_22 is not valid for the voltage channel and is rejected.
    /// @param gain PGA gain setting.
    /// @return ::ksfTkErrOk on success, ::ksfTkErrFail for an invalid gain, or an error code on
    /// communication failure.
    sfTkError_t setGainV(sfe_ade7953_pga_gain_t gain);

    /// @brief Get the PGA gain for the voltage channel.
    /// @param gain Output reference that receives the PGA gain setting.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getGainV(sfe_ade7953_pga_gain_t &gain);

    /// @brief Set the digital (fine) gain for Current Channel A using a raw register value.
    /// @param gain Digital gain register value (0x400000 = unity).
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t setDigitalGainIA(uint32_t gain);

    /// @brief Set the digital (fine) gain for Current Channel A using a floating-point multiplier.
    /// @details Converts the multiplier to the nearest valid register value (0x400000 = 1.0x).
    /// @param multiplier Desired gain multiplier (must be > 0 and within the representable range).
    /// @return ::ksfTkErrOk on success, ::ksfTkErrFail if out of range, or an error code on failure.
    sfTkError_t setDigitalGainIA(float multiplier);

    /// @brief Get the digital gain for Current Channel A as a raw register value.
    /// @param gain Output reference that receives the digital gain register value.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getDigitalGainIA(uint32_t &gain);

    /// @brief Get the digital gain for Current Channel A as a floating-point multiplier.
    /// @param multiplier Output reference that receives the gain multiplier (1.0 = unity).
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getDigitalGainIA(float &multiplier);

    /// @brief Set the digital (fine) gain for Current Channel B using a raw register value.
    /// @param gain Digital gain register value (0x400000 = unity).
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t setDigitalGainIB(uint32_t gain);

    /// @brief Set the digital (fine) gain for Current Channel B using a floating-point multiplier.
    /// @param multiplier Desired gain multiplier (must be > 0 and within the representable range).
    /// @return ::ksfTkErrOk on success, ::ksfTkErrFail if out of range, or an error code on failure.
    sfTkError_t setDigitalGainIB(float multiplier);

    /// @brief Get the digital gain for Current Channel B as a raw register value.
    /// @param gain Output reference that receives the digital gain register value.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getDigitalGainIB(uint32_t &gain);

    /// @brief Get the digital gain for Current Channel B as a floating-point multiplier.
    /// @param multiplier Output reference that receives the gain multiplier (1.0 = unity).
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getDigitalGainIB(float &multiplier);

    // ======================= Current Measurement ==========================

    /// @brief Read the raw IRMS value for Current Channel A.
    /// @param value Output reference that receives the raw unsigned IRMS value.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getIRmsA(uint32_t &value);

    /// @brief Read the raw IRMS value for Current Channel B.
    /// @param value Output reference that receives the raw unsigned IRMS value.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getIRmsB(uint32_t &value);

    /// @brief Read the RMS current on Channel A converted to amps.
    /// @details Combines the raw IRMS reading with the configured PGA gain, CT ratio, burden
    /// resistor, and full-scale constants. The PGA gain is read from the device on each call, so the
    /// conversion always matches the active gain.
    /// @param amps Output reference that receives the current in amps.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getCurrentA(float &amps);

    /// @brief Read the RMS current on Channel B converted to amps.
    /// @param amps Output reference that receives the current in amps.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getCurrentB(float &amps);

    /// @brief Read the instantaneous current sample for Channel A.
    /// @param value Output reference that receives the signed instantaneous current value.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getInstantaneousIA(int32_t &value);

    /// @brief Read the instantaneous current sample for Channel B.
    /// @param value Output reference that receives the signed instantaneous current value.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getInstantaneousIB(int32_t &value);

    // =================== Current Conversion Calibration ===================

    /// @brief Set the CT (current transformer) turns ratio used by getCurrentA() / getCurrentB().
    /// @param ratio Turns ratio of the current transformer (e.g. 2000.0 for a 30A:15mA clamp).
    void setCurrentTransformerRatio(float ratio);

    /// @brief Get the configured CT turns ratio.
    /// @return The current transformer turns ratio.
    float getCurrentTransformerRatio(void);

    /// @brief Set the burden resistor value (in ohms) used by getCurrentA() / getCurrentB().
    /// @param ohms Burden resistor value in ohms.
    void setBurdenResistor(float ohms);

    /// @brief Get the configured burden resistor value.
    /// @return The burden resistor value in ohms.
    float getBurdenResistor(void);

    // ======================== Peak Detection ==============================

    /// @brief Read the peak current value for Channel A.
    /// @param value Output reference that receives the peak value.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getPeakIA(uint32_t &value);

    /// @brief Read the peak current value for Channel B.
    /// @param value Output reference that receives the peak value.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getPeakIB(uint32_t &value);

    /// @brief Read the peak current value for Channel A and clear it.
    /// @details Reads the read-with-reset register, which returns the peak value and clears it in a
    /// single operation.
    /// @param value Output reference that receives the peak value before reset.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t readAndResetPeakIA(uint32_t &value);

    /// @brief Read the peak current value for Channel B and clear it.
    /// @param value Output reference that receives the peak value before reset.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t readAndResetPeakIB(uint32_t &value);

    /// @brief Set the overcurrent detection threshold.
    /// @param level Threshold level (24-bit value).
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t setOvercurrentLevel(uint32_t level);

    /// @brief Get the overcurrent detection threshold.
    /// @param level Output reference that receives the threshold level.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getOvercurrentLevel(uint32_t &level);

    // ====================== Calibration & Offset ==========================

    /// @brief Set the IRMS offset for Current Channel A.
    /// @note The ADE7953 applies this offset in the squared domain (the value is added to the
    /// square of the RMS result before the square root). To null a no-load noise floor of @c N raw
    /// counts the offset is approximately @c -(N*N) scaled per the datasheet, not @c -N.
    /// @param offset Signed offset value for IRMS correction.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t setIRmsOffsetA(int32_t offset);

    /// @brief Get the IRMS offset for Current Channel A.
    /// @param offset Output reference that receives the signed offset value.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getIRmsOffsetA(int32_t &offset);

    /// @brief Set the IRMS offset for Current Channel B. See setIRmsOffsetA() for the squared-domain note.
    /// @param offset Signed offset value for IRMS correction.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t setIRmsOffsetB(int32_t offset);

    /// @brief Get the IRMS offset for Current Channel B.
    /// @param offset Output reference that receives the signed offset value.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getIRmsOffsetB(int32_t &offset);

    // ======================= Zero-Crossing (ZX_I) =========================

    /// @brief Select which current channel drives the ZX_I output.
    /// @param useChannelB If true, ZX_I is based on Channel B; if false, Channel A.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t setZXISourceChannel(bool useChannelB);

    /// @brief Get which current channel drives the ZX_I output.
    /// @param useChannelB Output reference set true if Channel B drives ZX_I, false for Channel A.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getZXISourceChannel(bool &useChannelB);

    /// @brief Set the zero-crossing edge selection for interrupt generation.
    /// @param edge Edge selection (see sfe_ade7953_zx_edge_t).
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t setZXEdge(sfe_ade7953_zx_edge_t edge);

    /// @brief Get the current zero-crossing edge selection.
    /// @param edge Output reference that receives the edge selection value.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getZXEdge(sfe_ade7953_zx_edge_t &edge);

    /// @brief Enable or disable the zero-crossing low-pass filter.
    /// @param enable True to enable the LPF, false to disable.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t enableZXLPF(bool enable);

    /// @brief Enable or disable the high-pass filter on all channels.
    /// @details The HPF removes DC offset before RMS computation. It should be enabled for accurate
    /// IRMS readings, and is enabled by default after power-on reset and by begin().
    /// @param enable True to enable the HPF, false to disable.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t enableHPF(bool enable);

    /// @brief Check whether the high-pass filter is currently enabled.
    /// @param enabled Output reference set true if the HPF is enabled.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t isHPFEnabled(bool &enabled);

    /// @brief Read the line period derived from zero-crossing detection.
    /// @param period Output reference that receives the period register value.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getPeriod(uint16_t &period);

    // =========================== Interrupts ===============================

    /// @brief Set the interrupt enable mask for Channel A (and voltage).
    /// @param mask Interrupt enable bitfield struct.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t setInterruptEnableA(sfe_ade7953_irq_reg_t mask);

    /// @brief Get the interrupt enable mask for Channel A.
    /// @param mask Output reference that receives the interrupt enable bitfield.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getInterruptEnableA(sfe_ade7953_irq_reg_t &mask);

    /// @brief Read the interrupt status for Channel A (non-destructive).
    /// @param status Output reference that receives the interrupt status bitfield.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getInterruptStatusA(sfe_ade7953_irq_reg_t &status);

    /// @brief Read the interrupt status for Channel A and clear it.
    /// @details Reads the read-with-reset register, returning the status and clearing the flags in a
    /// single operation.
    /// @param status Output reference that receives the interrupt status bitfield before reset.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t readAndResetInterruptStatusA(sfe_ade7953_irq_reg_t &status);

    /// @brief Set the interrupt enable mask for Channel B.
    /// @details Only bits [13:0] are valid for Channel B.
    /// @param mask Interrupt enable bitfield struct.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t setInterruptEnableB(sfe_ade7953_irq_reg_t mask);

    /// @brief Get the interrupt enable mask for Channel B.
    /// @param mask Output reference that receives the interrupt enable bitfield.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getInterruptEnableB(sfe_ade7953_irq_reg_t &mask);

    /// @brief Read the interrupt status for Channel B (non-destructive).
    /// @param status Output reference that receives the interrupt status bitfield.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getInterruptStatusB(sfe_ade7953_irq_reg_t &status);

    /// @brief Read the interrupt status for Channel B and clear it.
    /// @param status Output reference that receives the interrupt status bitfield before reset.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t readAndResetInterruptStatusB(sfe_ade7953_irq_reg_t &status);

    // ========================= No-Load Detection ==========================

    /// @brief Set the no-load detection disable register.
    /// @param mask No-load disable bitfield struct.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t setNoLoadDisable(sfe_ade7953_disnoload_reg_t mask);

    /// @brief Get the no-load detection disable register.
    /// @param mask Output reference that receives the no-load disable bitfield.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getNoLoadDisable(sfe_ade7953_disnoload_reg_t &mask);

    // ====================== Line Cycle Accumulation =======================

    /// @brief Set the line cycle accumulation mode register.
    /// @param mode Line cycle mode bitfield struct.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t setLineCycleMode(sfe_ade7953_lcycmode_reg_t mode);

    /// @brief Get the line cycle accumulation mode register.
    /// @param mode Output reference that receives the line cycle mode bitfield.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getLineCycleMode(sfe_ade7953_lcycmode_reg_t &mode);

    /// @brief Set the number of half line cycles for line cycle accumulation.
    /// @param halfCycles Number of half cycles.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t setLineCycleCount(uint16_t halfCycles);

    /// @brief Get the number of half line cycles for line cycle accumulation.
    /// @param halfCycles Output reference that receives the number of half cycles.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getLineCycleCount(uint16_t &halfCycles);

    // ========================= Sag Detection ==============================

    /// @brief Set the number of sag line cycles.
    /// @param cycles Number of half line cycles for sag detection.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t setSagCycles(uint8_t cycles);

    /// @brief Get the number of sag line cycles.
    /// @param cycles Output reference that receives the number of half line cycles.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getSagCycles(uint8_t &cycles);

    /// @brief Set the sag voltage level threshold.
    /// @param level Sag level threshold (24-bit value).
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t setSagLevel(uint32_t level);

    /// @brief Get the sag voltage level threshold.
    /// @param level Output reference that receives the sag level.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getSagLevel(uint32_t &level);

    // ========================== Diagnostics ===============================

    /// @brief Get the type of the last successful communication.
    /// @param op Output reference that receives 0x35 for a read or 0xCA for a write.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getLastOperation(uint8_t &op);

    /// @brief Get the address of the last successful communication.
    /// @param address Output reference that receives the register address.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getLastAddress(uint16_t &address);

    /// @brief Get the data from the last successful 8-bit register communication.
    /// @param data Output reference that receives the data value.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getLastData8(uint8_t &data);

    /// @brief Get the data from the last successful 16-bit register communication.
    /// @param data Output reference that receives the data value.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getLastData16(uint16_t &data);

    /// @brief Get the data from the last successful 24/32-bit register communication.
    /// @param data Output reference that receives the data value.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getLastData32(uint32_t &data);

  protected:
    /// @brief Apply the datasheet unlock + optimize sequence and default configuration.
    /// @details Shared by begin() and reset(). Assumes _theBus is valid and byte order is set.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t applyDefaultConfig(void);

    /// @brief Convert a PGA gain enum value to its numeric multiplier (1, 2, 4, 8, 16, 22).
    static float pgaGainToMultiplier(sfe_ade7953_pga_gain_t gain);

    sfTkIBus *_theBus; ///< Pointer to the communication bus device.

    // --- Current-to-amps conversion parameters (sensible defaults for the Qwiic board) ---
    float _ctRatio = 2000.0f;          ///< CT turns ratio (30A:15mA = 2000:1).
    float _burdenResistor = 5.6f;      ///< Burden resistor in ohms.
    float _fullScaleCode = 5928256.0f; ///< ADE7953 full-scale IRMS code.
    float _fullScaleVrms = 0.35355f;   ///< Full-scale input RMS voltage (0.5 V peak / sqrt(2)).

    ///////////////////////////////////////////////////////////////////////////
    // I2C Addressing
    ///////////////////////////////////////////////////////////////////////////
    static const uint8_t kI2CAddress = 0x38; ///< 7-bit I2C address for the ADE7953 (only address).

    // 8-Bit Register Addresses (0x0xx)
    static const uint16_t kRegSagCyc = 0x000;       ///< Sag line cycles
    static const uint16_t kRegDisNoLoad = 0x001;    ///< No-load detection disable
    static const uint16_t kRegLCycMode = 0x004;     ///< Line cycle accumulation mode config
    static const uint16_t kRegPgaV = 0x007;         ///< Voltage channel PGA gain (Bits[2:0])
    static const uint16_t kRegPgaIA = 0x008;        ///< Current Channel A PGA gain (Bits[2:0])
    static const uint16_t kRegPgaIB = 0x009;        ///< Current Channel B PGA gain (Bits[2:0])
    static const uint16_t kRegWriteProtect = 0x040; ///< Write protection bits (Bits[2:0])
    static const uint16_t kRegLastOp = 0x0FD;       ///< Last operation type (0x35=read, 0xCA=write)
    static const uint16_t kRegLastRwData8 = 0x0FF;  ///< Last successful 8-bit register data
    static const uint16_t kRegVersion = 0x702;      ///< Silicon version number
    static const uint16_t kRegExRef = 0x800;        ///< Reference input config (0=internal, 1=external)
    static const uint16_t kRegUnlock = 0x0FE;       ///< Register unlock (write key to unlock 0x120)

    // 16-Bit Register Addresses (0x1xx)
    static const uint16_t kRegZXTout = 0x100;       ///< Zero-crossing timeout
    static const uint16_t kRegLineCyc = 0x101;      ///< Half line cycles for line cycle accumulation
    static const uint16_t kRegConfig = 0x102;       ///< Configuration register
    static const uint16_t kRegCF1Den = 0x103;       ///< CF1 frequency divider denominator
    static const uint16_t kRegCF2Den = 0x104;       ///< CF2 frequency divider denominator
    static const uint16_t kRegCFMode = 0x107;       ///< CF output selection
    static const uint16_t kRegPhCalA = 0x108;       ///< Phase calibration (Channel A)
    static const uint16_t kRegPhCalB = 0x109;       ///< Phase calibration (Channel B)
    static const uint16_t kRegPFA = 0x10A;          ///< Power factor (Channel A)
    static const uint16_t kRegPFB = 0x10B;          ///< Power factor (Channel B)
    static const uint16_t kRegAngleA = 0x10C;       ///< Angle between voltage and Current Channel A
    static const uint16_t kRegAngleB = 0x10D;       ///< Angle between voltage and Current Channel B
    static const uint16_t kRegPeriod = 0x10E;       ///< Period register (line period from ZX)
    static const uint16_t kRegAltOutput = 0x110;    ///< Alternative output functions
    static const uint16_t kRegOptimize = 0x120;     ///< Performance optimization (set to 0x0030)
    static const uint16_t kRegLastAdd = 0x1FE;      ///< Last successful communication address
    static const uint16_t kRegLastRwData16 = 0x1FF; ///< Last successful 16-bit register data

    // 32-Bit Register Addresses (0x3xx) -- valid data in lower 24 bits, upper 8 sign-extended
    static const uint16_t kRegSagLvl = 0x300;    ///< Sag voltage level
    static const uint16_t kRegAccMode = 0x301;   ///< Accumulation mode
    static const uint16_t kRegApNoLoad = 0x303;  ///< Active power no-load level
    static const uint16_t kRegVarNoLoad = 0x304; ///< Reactive power no-load level
    static const uint16_t kRegVaNoLoad = 0x305;  ///< Apparent power no-load level
    static const uint16_t kRegAVA = 0x310;       ///< Instantaneous apparent power (Channel A)
    static const uint16_t kRegBVA = 0x311;       ///< Instantaneous apparent power (Channel B)
    static const uint16_t kRegAWatt = 0x312;     ///< Instantaneous active power (Channel A)
    static const uint16_t kRegBWatt = 0x313;     ///< Instantaneous active power (Channel B)
    static const uint16_t kRegAVar = 0x314;      ///< Instantaneous reactive power (Channel A)
    static const uint16_t kRegBVar = 0x315;      ///< Instantaneous reactive power (Channel B)
    static const uint16_t kRegIA = 0x316;        ///< Instantaneous current (Channel A)
    static const uint16_t kRegIB = 0x317;        ///< Instantaneous current (Channel B)
    static const uint16_t kRegV = 0x318;         ///< Instantaneous voltage
    static const uint16_t kRegIRmsA = 0x31A;     ///< IRMS (Channel A)
    static const uint16_t kRegIRmsB = 0x31B;     ///< IRMS (Channel B)
    static const uint16_t kRegVRms = 0x31C;      ///< VRMS
    static const uint16_t kRegAEnergyA = 0x31E;  ///< Active energy (Channel A)
    static const uint16_t kRegAEnergyB = 0x31F;  ///< Active energy (Channel B)
    static const uint16_t kRegREnergyA = 0x320;  ///< Reactive energy (Channel A)
    static const uint16_t kRegREnergyB = 0x321;  ///< Reactive energy (Channel B)
    static const uint16_t kRegApEnergyA = 0x322; ///< Apparent energy (Channel A)
    static const uint16_t kRegApEnergyB = 0x323; ///< Apparent energy (Channel B)
    static const uint16_t kRegOvLvl = 0x324;     ///< Overvoltage level
    static const uint16_t kRegOiLvl = 0x325;     ///< Overcurrent level
    static const uint16_t kRegVPeak = 0x326;     ///< Voltage channel peak
    static const uint16_t kRegRstVPeak = 0x327;  ///< Voltage peak read with reset
    static const uint16_t kRegIAPeak = 0x328;    ///< Current Channel A peak
    static const uint16_t kRegRstIAPeak = 0x329; ///< Current Channel A peak read with reset
    static const uint16_t kRegIBPeak = 0x32A;    ///< Current Channel B peak
    static const uint16_t kRegRstIBPeak = 0x32B; ///< Current Channel B peak read with reset
    static const uint16_t kRegIrqEnA = 0x32C;    ///< Interrupt enable (Channel A + voltage)
    static const uint16_t kRegIrqStatA = 0x32D;  ///< Interrupt status (Channel A + voltage)
    static const uint16_t kRegRstIrqStatA = 0x32E; ///< Reset interrupt status (Channel A + voltage)
    static const uint16_t kRegIrqEnB = 0x32F;    ///< Interrupt enable (Channel B)
    static const uint16_t kRegIrqStatB = 0x330;  ///< Interrupt status (Channel B)
    static const uint16_t kRegRstIrqStatB = 0x331; ///< Reset interrupt status (Channel B)
    static const uint16_t kRegCRC = 0x37F;       ///< Checksum (32-bit only)
    static const uint16_t kRegAIGain = 0x380;    ///< Current channel gain (Channel A)
    static const uint16_t kRegAVGain = 0x381;    ///< Voltage channel gain
    static const uint16_t kRegAWGain = 0x382;    ///< Active power gain (Channel A)
    static const uint16_t kRegAVarGain = 0x383;  ///< Reactive power gain (Channel A)
    static const uint16_t kRegAVaGain = 0x384;   ///< Apparent power gain (Channel A)
    static const uint16_t kRegAIRmsOS = 0x386;   ///< IRMS offset (Channel A)
    static const uint16_t kRegVRmsOS = 0x388;    ///< VRMS offset
    static const uint16_t kRegAWattOS = 0x389;   ///< Active power offset (Channel A)
    static const uint16_t kRegAVarOS = 0x38A;    ///< Reactive power offset (Channel A)
    static const uint16_t kRegAVaOS = 0x38B;     ///< Apparent power offset (Channel A)
    static const uint16_t kRegBIGain = 0x38C;    ///< Current channel gain (Channel B)
    static const uint16_t kRegBWGain = 0x38E;    ///< Active power gain (Channel B)
    static const uint16_t kRegBVarGain = 0x38F;  ///< Reactive power gain (Channel B)
    static const uint16_t kRegBVaGain = 0x390;   ///< Apparent power gain (Channel B)
    static const uint16_t kRegBIRmsOS = 0x392;   ///< IRMS offset (Channel B)
    static const uint16_t kRegBWattOS = 0x395;   ///< Active power offset (Channel B)
    static const uint16_t kRegBVarOS = 0x396;    ///< Reactive power offset (Channel B)
    static const uint16_t kRegBVaOS = 0x397;     ///< Apparent power offset (Channel B)
    static const uint16_t kRegLastRwData32 = 0x3FF; ///< Last successful 24/32-bit register data

    // Magic Values
    static const uint8_t kUnlockKey = 0xAD;             ///< Key written to kRegUnlock to unlock 0x120.
    static const uint16_t kOptimizeValue = 0x0030;      ///< Value written to 0x120 for rated performance.
    static const uint32_t kDigitalGainUnity = 0x400000; ///< Digital gain register value for 1.0x.
    static const uint32_t kDigitalGainMax = 0x7FFFFF;   ///< Maximum valid digital gain register value.
    static const uint8_t kLastOpRead = 0x35;            ///< Last operation was a read.
    static const uint8_t kLastOpWrite = 0xCA;           ///< Last operation was a write.
};
