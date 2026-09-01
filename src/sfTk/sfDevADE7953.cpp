/**
 * @file sfDevADE7953.cpp
 * @brief Implementation file for the SparkFun ADE7953 Energy Meter Driver.
 *
 * @details
 * This file implements the sfDevADE7953 class methods for configuring and reading data from
 * the ADE7953 energy metering IC. The driver provides a comms-agnostic interface using the
 * SparkFun Toolkit, focused on current measurement via CT clamp with support for gain
 * configuration, zero-crossing detection, peak measurement, and interrupt handling.
 *
 * @author SparkFun Electronics
 * @date 2025
 * @copyright Copyright (c) 2025, SparkFun Electronics Inc. This project is released under the MIT License.
 *
 * SPDX-License-Identifier: MIT
 *
 * @see https://github.com/sparkfun/SparkFun_ADE7953_Arduino_Library
 */

#include "sfDevADE7953.h"

#include <math.h>

// ========================= Setup & Identity ===============================

sfTkError_t sfDevADE7953::begin(sfTkIBus *theBus)
{
    // Adopt the supplied bus if one was provided; otherwise keep any bus set by a prior begin().
    if (theBus != nullptr)
        _theBus = theBus;

    // We need a bus to talk to.
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    // The ADE7953 is big-endian for both register addresses and data.
    _theBus->setByteOrder(sfTkByteOrder::BigEndian);

    // Make sure the device is actually present before writing any configuration. This avoids
    // clobbering registers on a different device that happens to share the bus address.
    if (!isConnected())
        return ksfTkErrBusNoResponse;

    // Apply the datasheet performance sequence and sensible defaults.
    return applyDefaultConfig();
}

sfTkError_t sfDevADE7953::applyDefaultConfig(void)
{
    // Unlock the optimized performance register by writing the key to register 0x0FE.
    sfTkError_t rc = _theBus->writeRegister(kRegUnlock, kUnlockKey);
    if (rc != ksfTkErrOk)
        return rc;

    // Write 0x0030 to register 0x120 for optimal performance per datasheet Table 1.
    rc = _theBus->writeRegister(kRegOptimize, kOptimizeValue);
    if (rc != ksfTkErrOk)
        return rc;

    // Default the current channels to 16x gain for maximum sensitivity with a CT clamp. A basic
    // sketch should call autoCalibrateA()/autoCalibrateB() at startup (clamp open, no load) to
    // zero the noise floor before reading current.
    rc = setGainIA(ADE7953_PGA_GAIN_16);
    if (rc != ksfTkErrOk)
        return rc;

    rc = setGainIB(ADE7953_PGA_GAIN_16);
    if (rc != ksfTkErrOk)
        return rc;

    // Enable the high-pass filter so IRMS readings are not corrupted by DC offset.
    return enableHPF(true);
}

bool sfDevADE7953::isConnected(void)
{
    uint8_t version = 0;
    if (getVersion(version) != ksfTkErrOk)
        return false;

    // A valid ADE7953 reports a nonzero silicon version.
    return version != 0;
}

sfTkError_t sfDevADE7953::getVersion(uint8_t &version)
{
    return _theBus->readRegister(kRegVersion, version);
}

sfTkError_t sfDevADE7953::reset(void)
{
    sfe_ade7953_config_reg_t config = {};

    sfTkError_t rc = _theBus->readRegister(kRegConfig, config.word);
    if (rc != ksfTkErrOk)
        return rc;

    config.swRst = 1;

    rc = _theBus->writeRegister(kRegConfig, config.word);
    if (rc != ksfTkErrOk)
        return rc;

    // A software reset returns every register to its power-on default, including the OPTIMIZE
    // register, so re-apply the performance sequence and default configuration. Callers that reset
    // a running device may wish to allow a short settling delay before resuming reads.
    return applyDefaultConfig();
}

sfTkError_t sfDevADE7953::setWriteProtect(uint8_t protect)
{
    return _theBus->writeRegister(kRegWriteProtect, (uint8_t)(protect & 0x07));
}

sfTkError_t sfDevADE7953::getWriteProtect(uint8_t &protect)
{
    sfTkError_t rc = _theBus->readRegister(kRegWriteProtect, protect);
    protect &= 0x07;
    return rc;
}

// ======================== PGA Gain Configuration ==========================

float sfDevADE7953::pgaGainToMultiplier(sfe_ade7953_pga_gain_t gain)
{
    switch (gain)
    {
    case ADE7953_PGA_GAIN_1:
        return 1.0f;
    case ADE7953_PGA_GAIN_2:
        return 2.0f;
    case ADE7953_PGA_GAIN_4:
        return 4.0f;
    case ADE7953_PGA_GAIN_8:
        return 8.0f;
    case ADE7953_PGA_GAIN_16:
        return 16.0f;
    case ADE7953_PGA_GAIN_22:
        return 22.0f;
    default:
        return 1.0f;
    }
}

sfTkError_t sfDevADE7953::setGainIA(sfe_ade7953_pga_gain_t gain)
{
    return _theBus->writeRegister(kRegPgaIA, (uint8_t)(gain & 0x07));
}

sfTkError_t sfDevADE7953::getGainIA(sfe_ade7953_pga_gain_t &gain)
{
    uint8_t value = 0;
    sfTkError_t rc = _theBus->readRegister(kRegPgaIA, value);
    gain = (sfe_ade7953_pga_gain_t)(value & 0x07);
    return rc;
}

sfTkError_t sfDevADE7953::setGainIB(sfe_ade7953_pga_gain_t gain)
{
    return _theBus->writeRegister(kRegPgaIB, (uint8_t)(gain & 0x07));
}

sfTkError_t sfDevADE7953::getGainIB(sfe_ade7953_pga_gain_t &gain)
{
    uint8_t value = 0;
    sfTkError_t rc = _theBus->readRegister(kRegPgaIB, value);
    gain = (sfe_ade7953_pga_gain_t)(value & 0x07);
    return rc;
}

sfTkError_t sfDevADE7953::setGainV(sfe_ade7953_pga_gain_t gain)
{
    // Gain code 5 (22x) is only valid for the current channels, not the voltage channel.
    if (gain == ADE7953_PGA_GAIN_22)
        return ksfTkErrFail;

    return _theBus->writeRegister(kRegPgaV, (uint8_t)(gain & 0x07));
}

sfTkError_t sfDevADE7953::getGainV(sfe_ade7953_pga_gain_t &gain)
{
    uint8_t value = 0;
    sfTkError_t rc = _theBus->readRegister(kRegPgaV, value);
    gain = (sfe_ade7953_pga_gain_t)(value & 0x07);
    return rc;
}

sfTkError_t sfDevADE7953::setDigitalGainIA(uint32_t gain)
{
    return _theBus->writeRegister(kRegAIGain, gain);
}

sfTkError_t sfDevADE7953::setDigitalGainIA(float multiplier)
{
    if (multiplier <= 0.0f)
        return ksfTkErrFail;

    uint32_t code = (uint32_t)((float)kDigitalGainUnity * multiplier + 0.5f);
    if (code > kDigitalGainMax)
        return ksfTkErrFail;

    return setDigitalGainIA(code);
}

sfTkError_t sfDevADE7953::getDigitalGainIA(uint32_t &gain)
{
    return _theBus->readRegister(kRegAIGain, gain);
}

sfTkError_t sfDevADE7953::getDigitalGainIA(float &multiplier)
{
    uint32_t code = 0;
    sfTkError_t rc = getDigitalGainIA(code);
    multiplier = (float)code / (float)kDigitalGainUnity;
    return rc;
}

sfTkError_t sfDevADE7953::setDigitalGainIB(uint32_t gain)
{
    return _theBus->writeRegister(kRegBIGain, gain);
}

sfTkError_t sfDevADE7953::setDigitalGainIB(float multiplier)
{
    if (multiplier <= 0.0f)
        return ksfTkErrFail;

    uint32_t code = (uint32_t)((float)kDigitalGainUnity * multiplier + 0.5f);
    if (code > kDigitalGainMax)
        return ksfTkErrFail;

    return setDigitalGainIB(code);
}

sfTkError_t sfDevADE7953::getDigitalGainIB(uint32_t &gain)
{
    return _theBus->readRegister(kRegBIGain, gain);
}

sfTkError_t sfDevADE7953::getDigitalGainIB(float &multiplier)
{
    uint32_t code = 0;
    sfTkError_t rc = getDigitalGainIB(code);
    multiplier = (float)code / (float)kDigitalGainUnity;
    return rc;
}

// ======================= Current Measurement ==============================

sfTkError_t sfDevADE7953::getIRMSA(uint32_t &value)
{
    return _theBus->readRegister(kRegIRMSA, value);
}

sfTkError_t sfDevADE7953::getIRMSB(uint32_t &value)
{
    return _theBus->readRegister(kRegIRMSB, value);
}

sfTkError_t sfDevADE7953::getCurrentA(float &amps)
{
    uint32_t raw = 0;
    sfTkError_t rc = getIRMSA(raw);
    if (rc != ksfTkErrOk)
        return rc;

    sfe_ade7953_pga_gain_t gain = ADE7953_PGA_GAIN_1;
    rc = getGainIA(gain);
    if (rc != ksfTkErrOk)
        return rc;

    // Remove the no-load baseline (if calibrated) in the squared domain. RMS values combine in
    // quadrature, so the correct way to strip a noise floor is sqrt(reading^2 - baseline^2).
    float corrected = removeBaseline(raw, _baselineA);

    // Pin voltage RMS = corrected / fullScaleCode * (fullScaleVRMS / pgaGain)
    // Secondary current  = pinVRMS / burdenResistor
    // Primary current    = secondaryCurrent * ctRatio
    float fullScaleAmps = (_fullScaleVRMS / pgaGainToMultiplier(gain)) / _burdenResistor * _ctRatioA;
    amps = corrected / _fullScaleCode * fullScaleAmps;
    return ksfTkErrOk;
}

sfTkError_t sfDevADE7953::getCurrentB(float &amps)
{
    uint32_t raw = 0;
    sfTkError_t rc = getIRMSB(raw);
    if (rc != ksfTkErrOk)
        return rc;

    sfe_ade7953_pga_gain_t gain = ADE7953_PGA_GAIN_1;
    rc = getGainIB(gain);
    if (rc != ksfTkErrOk)
        return rc;

    float corrected = removeBaseline(raw, _baselineB);

    float fullScaleAmps = (_fullScaleVRMS / pgaGainToMultiplier(gain)) / _burdenResistor * _ctRatioB;
    amps = corrected / _fullScaleCode * fullScaleAmps;
    return ksfTkErrOk;
}

sfTkError_t sfDevADE7953::getInstantaneousIA(int32_t &value)
{
    uint32_t raw = 0;
    sfTkError_t rc = _theBus->readRegister(kRegIA, raw);

    // The upper 8 bits from the 0x3xx read are already sign-extended by the ADE7953, so a plain
    // conversion to a signed 32-bit value preserves the sign.
    value = (int32_t)raw;
    return rc;
}

sfTkError_t sfDevADE7953::getInstantaneousIB(int32_t &value)
{
    uint32_t raw = 0;
    sfTkError_t rc = _theBus->readRegister(kRegIB, raw);
    value = (int32_t)raw;
    return rc;
}

// =================== Current Conversion Calibration =======================

float sfDevADE7953::removeBaseline(uint32_t reading, uint32_t baseline)
{
    if (baseline == 0)
        return (float)reading;

    if (reading <= baseline)
        return 0.0f;

    float r = (float)reading;
    float b = (float)baseline;
    return sqrtf(r * r - b * b);
}

void sfDevADE7953::clampPreset(sfe_ade7953_clamp_t clamp, float &ratio, sfe_ade7953_pga_gain_t &gain)
{
    switch (clamp)
    {
    case ADE7953_CLAMP_SCT013:
        ratio = 2000.0f;
        gain = ADE7953_PGA_GAIN_16;
        break;

    case ADE7953_CLAMP_ECS1030:
    default:
        ratio = 2000.0f;
        gain = ADE7953_PGA_GAIN_16;
        break;
    }
}

sfTkError_t sfDevADE7953::setCurrentClamp(sfe_ade7953_clamp_t clamp)
{
    float ratio = 2000.0f;
    sfe_ade7953_pga_gain_t gain = ADE7953_PGA_GAIN_16;
    clampPreset(clamp, ratio, gain);

    _ctRatioA = ratio;
    _ctRatioB = ratio;

    sfTkError_t rc = setGainIA(gain);
    if (rc != ksfTkErrOk)
        return rc;

    return setGainIB(gain);
}

sfTkError_t sfDevADE7953::setCurrentClampA(sfe_ade7953_clamp_t clamp)
{
    float ratio = 2000.0f;
    sfe_ade7953_pga_gain_t gain = ADE7953_PGA_GAIN_16;
    clampPreset(clamp, ratio, gain);

    _ctRatioA = ratio;
    return setGainIA(gain);
}

sfTkError_t sfDevADE7953::setCurrentClampB(sfe_ade7953_clamp_t clamp)
{
    float ratio = 2000.0f;
    sfe_ade7953_pga_gain_t gain = ADE7953_PGA_GAIN_16;
    clampPreset(clamp, ratio, gain);

    _ctRatioB = ratio;
    return setGainIB(gain);
}

void sfDevADE7953::setCurrentClamp(float turnsRatio)
{
    _ctRatioA = turnsRatio;
    _ctRatioB = turnsRatio;
}

void sfDevADE7953::setCurrentClampA(float turnsRatio)
{
    _ctRatioA = turnsRatio;
}

void sfDevADE7953::setCurrentClampB(float turnsRatio)
{
    _ctRatioB = turnsRatio;
}

sfTkError_t sfDevADE7953::autoCalibrateA(uint16_t numSamples)
{
    if (numSamples == 0)
        return ksfTkErrFail;

    uint64_t sum = 0;
    for (uint16_t i = 0; i < numSamples; i++)
    {
        uint32_t sample = 0;
        sfTkError_t rc = getIRMSA(sample);
        if (rc != ksfTkErrOk)
            return rc;
        sum += sample;
    }

    _baselineA = (uint32_t)(sum / numSamples);
    return ksfTkErrOk;
}

sfTkError_t sfDevADE7953::autoCalibrateB(uint16_t numSamples)
{
    if (numSamples == 0)
        return ksfTkErrFail;

    uint64_t sum = 0;
    for (uint16_t i = 0; i < numSamples; i++)
    {
        uint32_t sample = 0;
        sfTkError_t rc = getIRMSB(sample);
        if (rc != ksfTkErrOk)
            return rc;
        sum += sample;
    }

    _baselineB = (uint32_t)(sum / numSamples);
    return ksfTkErrOk;
}

void sfDevADE7953::clearCalibration(void)
{
    _baselineA = 0;
    _baselineB = 0;
}

void sfDevADE7953::setCurrentTransformerRatio(float ratio)
{
    _ctRatioA = ratio;
    _ctRatioB = ratio;
}

void sfDevADE7953::setCurrentTransformerRatioA(float ratio)
{
    _ctRatioA = ratio;
}

void sfDevADE7953::setCurrentTransformerRatioB(float ratio)
{
    _ctRatioB = ratio;
}

float sfDevADE7953::getCurrentTransformerRatio(void)
{
    return _ctRatioA;
}

float sfDevADE7953::getCurrentTransformerRatioA(void)
{
    return _ctRatioA;
}

float sfDevADE7953::getCurrentTransformerRatioB(void)
{
    return _ctRatioB;
}

void sfDevADE7953::setBurdenResistor(float ohms)
{
    _burdenResistor = ohms;
}

float sfDevADE7953::getBurdenResistor(void)
{
    return _burdenResistor;
}

// ======================== Peak Detection ==================================

sfTkError_t sfDevADE7953::getPeakIA(uint32_t &value)
{
    return _theBus->readRegister(kRegIAPeak, value);
}

sfTkError_t sfDevADE7953::getPeakIB(uint32_t &value)
{
    return _theBus->readRegister(kRegIBPeak, value);
}

sfTkError_t sfDevADE7953::readAndResetPeakIA(uint32_t &value)
{
    // Reading RSTIAPEAK returns the peak value and clears the register in a single operation.
    return _theBus->readRegister(kRegRstIAPeak, value);
}

sfTkError_t sfDevADE7953::readAndResetPeakIB(uint32_t &value)
{
    return _theBus->readRegister(kRegRstIBPeak, value);
}

sfTkError_t sfDevADE7953::setOvercurrentLevel(uint32_t level)
{
    return _theBus->writeRegister(kRegOiLvl, level);
}

sfTkError_t sfDevADE7953::getOvercurrentLevel(uint32_t &level)
{
    return _theBus->readRegister(kRegOiLvl, level);
}

// ====================== Calibration & Offset ==============================

sfTkError_t sfDevADE7953::setIRMSOffsetA(int32_t offset)
{
    return _theBus->writeRegister(kRegAIRMSOS, (uint32_t)offset);
}

sfTkError_t sfDevADE7953::getIRMSOffsetA(int32_t &offset)
{
    uint32_t raw = 0;
    sfTkError_t rc = _theBus->readRegister(kRegAIRMSOS, raw);
    offset = (int32_t)raw;
    return rc;
}

sfTkError_t sfDevADE7953::setIRMSOffsetB(int32_t offset)
{
    return _theBus->writeRegister(kRegBIRMSOS, (uint32_t)offset);
}

sfTkError_t sfDevADE7953::getIRMSOffsetB(int32_t &offset)
{
    uint32_t raw = 0;
    sfTkError_t rc = _theBus->readRegister(kRegBIRMSOS, raw);
    offset = (int32_t)raw;
    return rc;
}

// ======================= Zero-Crossing (ZX_I) =============================

sfTkError_t sfDevADE7953::setZXISourceChannel(bool useChannelB)
{
    sfe_ade7953_config_reg_t config = {};
    sfTkError_t rc = _theBus->readRegister(kRegConfig, config.word);
    if (rc != ksfTkErrOk)
        return rc;

    config.zxI = useChannelB ? 1 : 0;

    return _theBus->writeRegister(kRegConfig, config.word);
}

sfTkError_t sfDevADE7953::getZXISourceChannel(bool &useChannelB)
{
    sfe_ade7953_config_reg_t config = {};
    sfTkError_t rc = _theBus->readRegister(kRegConfig, config.word);
    useChannelB = config.zxI != 0;
    return rc;
}

sfTkError_t sfDevADE7953::setZXEdge(sfe_ade7953_zx_edge_t edge)
{
    sfe_ade7953_config_reg_t config = {};
    sfTkError_t rc = _theBus->readRegister(kRegConfig, config.word);
    if (rc != ksfTkErrOk)
        return rc;

    config.zxEdge = (uint16_t)edge;

    return _theBus->writeRegister(kRegConfig, config.word);
}

sfTkError_t sfDevADE7953::getZXEdge(sfe_ade7953_zx_edge_t &edge)
{
    sfe_ade7953_config_reg_t config = {};
    sfTkError_t rc = _theBus->readRegister(kRegConfig, config.word);
    edge = (sfe_ade7953_zx_edge_t)config.zxEdge;
    return rc;
}

sfTkError_t sfDevADE7953::enableZXLPF(bool enable)
{
    sfe_ade7953_config_reg_t config = {};
    sfTkError_t rc = _theBus->readRegister(kRegConfig, config.word);
    if (rc != ksfTkErrOk)
        return rc;

    // Note: the ZXLPF bit is inverted — 0 means the LPF is ENABLED, 1 means DISABLED.
    config.zxLPF = enable ? 0 : 1;

    return _theBus->writeRegister(kRegConfig, config.word);
}

sfTkError_t sfDevADE7953::enableHPF(bool enable)
{
    sfe_ade7953_config_reg_t config = {};
    sfTkError_t rc = _theBus->readRegister(kRegConfig, config.word);
    if (rc != ksfTkErrOk)
        return rc;

    config.hpfEn = enable ? 1 : 0;

    return _theBus->writeRegister(kRegConfig, config.word);
}

sfTkError_t sfDevADE7953::isHPFEnabled(bool &enabled)
{
    sfe_ade7953_config_reg_t config = {};
    sfTkError_t rc = _theBus->readRegister(kRegConfig, config.word);
    enabled = config.hpfEn != 0;
    return rc;
}

sfTkError_t sfDevADE7953::getPeriod(uint16_t &period)
{
    return _theBus->readRegister(kRegPeriod, period);
}

// =========================== Interrupts ===================================

sfTkError_t sfDevADE7953::setInterruptEnableA(sfe_ade7953_irq_reg_t mask)
{
    return _theBus->writeRegister(kRegIrqEnA, mask.word);
}

sfTkError_t sfDevADE7953::getInterruptEnableA(sfe_ade7953_irq_reg_t &mask)
{
    return _theBus->readRegister(kRegIrqEnA, mask.word);
}

sfTkError_t sfDevADE7953::getInterruptStatusA(sfe_ade7953_irq_reg_t &status)
{
    return _theBus->readRegister(kRegIrqStatA, status.word);
}

sfTkError_t sfDevADE7953::readAndResetInterruptStatusA(sfe_ade7953_irq_reg_t &status)
{
    // Reading RSTIRQSTATA returns the status and clears the flags in a single operation.
    return _theBus->readRegister(kRegRstIrqStatA, status.word);
}

sfTkError_t sfDevADE7953::setInterruptEnableB(sfe_ade7953_irq_reg_t mask)
{
    return _theBus->writeRegister(kRegIrqEnB, mask.word);
}

sfTkError_t sfDevADE7953::getInterruptEnableB(sfe_ade7953_irq_reg_t &mask)
{
    return _theBus->readRegister(kRegIrqEnB, mask.word);
}

sfTkError_t sfDevADE7953::getInterruptStatusB(sfe_ade7953_irq_reg_t &status)
{
    return _theBus->readRegister(kRegIrqStatB, status.word);
}

sfTkError_t sfDevADE7953::readAndResetInterruptStatusB(sfe_ade7953_irq_reg_t &status)
{
    return _theBus->readRegister(kRegRstIrqStatB, status.word);
}

// ========================= No-Load Detection ==============================

sfTkError_t sfDevADE7953::setNoLoadDisable(sfe_ade7953_disnoload_reg_t mask)
{
    return _theBus->writeRegister(kRegDisNoLoad, mask.byte);
}

sfTkError_t sfDevADE7953::getNoLoadDisable(sfe_ade7953_disnoload_reg_t &mask)
{
    return _theBus->readRegister(kRegDisNoLoad, mask.byte);
}

// ====================== Line Cycle Accumulation ===========================

sfTkError_t sfDevADE7953::setLineCycleMode(sfe_ade7953_lcycmode_reg_t mode)
{
    return _theBus->writeRegister(kRegLCycMode, mode.byte);
}

sfTkError_t sfDevADE7953::getLineCycleMode(sfe_ade7953_lcycmode_reg_t &mode)
{
    return _theBus->readRegister(kRegLCycMode, mode.byte);
}

sfTkError_t sfDevADE7953::setLineCycleCount(uint16_t halfCycles)
{
    return _theBus->writeRegister(kRegLineCyc, halfCycles);
}

sfTkError_t sfDevADE7953::getLineCycleCount(uint16_t &halfCycles)
{
    return _theBus->readRegister(kRegLineCyc, halfCycles);
}

// ========================= Sag Detection ==================================

sfTkError_t sfDevADE7953::setSagCycles(uint8_t cycles)
{
    return _theBus->writeRegister(kRegSagCyc, cycles);
}

sfTkError_t sfDevADE7953::getSagCycles(uint8_t &cycles)
{
    return _theBus->readRegister(kRegSagCyc, cycles);
}

sfTkError_t sfDevADE7953::setSagLevel(uint32_t level)
{
    return _theBus->writeRegister(kRegSagLvl, level);
}

sfTkError_t sfDevADE7953::getSagLevel(uint32_t &level)
{
    return _theBus->readRegister(kRegSagLvl, level);
}

// ========================== Diagnostics ===================================

sfTkError_t sfDevADE7953::getLastOperation(uint8_t &op)
{
    return _theBus->readRegister(kRegLastOp, op);
}

sfTkError_t sfDevADE7953::getLastAddress(uint16_t &address)
{
    return _theBus->readRegister(kRegLastAdd, address);
}

sfTkError_t sfDevADE7953::getLastData8(uint8_t &data)
{
    return _theBus->readRegister(kRegLastRwData8, data);
}

sfTkError_t sfDevADE7953::getLastData16(uint16_t &data)
{
    return _theBus->readRegister(kRegLastRwData16, data);
}

sfTkError_t sfDevADE7953::getLastData32(uint32_t &data)
{
    return _theBus->readRegister(kRegLastRwData32, data);
}
