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
 * @copyright Copyright (c) 2025, SparkFun Electronics Inc. All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * @see https://github.com/sparkfun/SparkFun_ADE7953_Arduino_Library
 */

#include "sfDevADE7953.h"

#define DEBUG_SERIAL_PRINTS (0)
#if DEBUG_SERIAL_PRINTS
#include "Arduino.h"
#endif

// ========================= Setup & Identity ===============================

bool sfDevADE7953::begin(sfTkIBus *theBus)
{
    // Nullptr check.
    if (!_theBus && !theBus)
        return false;

    // Set the internal bus pointer, overriding current bus if it exists.
    if (theBus != nullptr)
        setCommunicationBus(theBus);

    // ADE7953 is big-endian for both register addresses and data.
    _theBus->setByteOrder(sfTkByteOrder::BigEndian);

    // Unlock the optimized performance register by writing 0xAD to register 0x0FE.
    if (_theBus->writeRegister(ksfADE7953RegUnlock, ksfADE7953UnlockKey) != ksfTkErrOk)
        return false;

    // Write 0x0030 to register 0x120 for optimal performance per datasheet Table 1.
    if (_theBus->writeRegister(ksfADE7953RegOptimize, ksfADE7953OptimizeValue) != ksfTkErrOk)
        return false;

    return true;
}

void sfDevADE7953::setCommunicationBus(sfTkIBus *theBus)
{
    _theBus = theBus;
}

uint8_t sfDevADE7953::getVersion(void)
{
    uint8_t version = 0;

    if (!_theBus)
        return 0;

    if (_theBus->readRegister(ksfADE7953RegVersion, version) != ksfTkErrOk)
        return 0;

    return version;
}

bool sfDevADE7953::reset(void)
{
    if (!_theBus)
        return false;

    // Read current CONFIG, set the SWRST bit, and write back.
    uint16_t config = 0;
    if (_theBus->readRegister(ksfADE7953RegConfig, config) != ksfTkErrOk)
        return false;

    config |= ksfADE7953ConfigSWRst;

    return (_theBus->writeRegister(ksfADE7953RegConfig, config) == ksfTkErrOk);
}

bool sfDevADE7953::setWriteProtect(uint8_t protect)
{
    if (!_theBus)
        return false;

    return (_theBus->writeRegister(ksfADE7953RegWriteProtect, (uint8_t)(protect & 0x07)) == ksfTkErrOk);
}

uint8_t sfDevADE7953::getWriteProtect(void)
{
    uint8_t value = 0;

    if (!_theBus)
        return 0;

    if (_theBus->readRegister(ksfADE7953RegWriteProtect, value) != ksfTkErrOk)
        return 0;

    return value & 0x07;
}

// ======================== PGA Gain Configuration ==========================

bool sfDevADE7953::setGainIA(sfe_ade7953_pga_gain_t gain)
{
    if (!_theBus)
        return false;

    return (_theBus->writeRegister(ksfADE7953RegPgaIA, (uint8_t)(gain & 0x07)) == ksfTkErrOk);
}

sfe_ade7953_pga_gain_t sfDevADE7953::getGainIA(void)
{
    uint8_t value = 0;

    if (!_theBus)
        return ADE7953_PGA_GAIN_1;

    if (_theBus->readRegister(ksfADE7953RegPgaIA, value) != ksfTkErrOk)
        return ADE7953_PGA_GAIN_1;

    return (sfe_ade7953_pga_gain_t)(value & 0x07);
}

bool sfDevADE7953::setGainIB(sfe_ade7953_pga_gain_t gain)
{
    if (!_theBus)
        return false;

    return (_theBus->writeRegister(ksfADE7953RegPgaIB, (uint8_t)(gain & 0x07)) == ksfTkErrOk);
}

sfe_ade7953_pga_gain_t sfDevADE7953::getGainIB(void)
{
    uint8_t value = 0;

    if (!_theBus)
        return ADE7953_PGA_GAIN_1;

    if (_theBus->readRegister(ksfADE7953RegPgaIB, value) != ksfTkErrOk)
        return ADE7953_PGA_GAIN_1;

    return (sfe_ade7953_pga_gain_t)(value & 0x07);
}

bool sfDevADE7953::setGainV(sfe_ade7953_pga_gain_t gain)
{
    if (!_theBus)
        return false;

    return (_theBus->writeRegister(ksfADE7953RegPgaV, (uint8_t)(gain & 0x07)) == ksfTkErrOk);
}

sfe_ade7953_pga_gain_t sfDevADE7953::getGainV(void)
{
    uint8_t value = 0;

    if (!_theBus)
        return ADE7953_PGA_GAIN_1;

    if (_theBus->readRegister(ksfADE7953RegPgaV, value) != ksfTkErrOk)
        return ADE7953_PGA_GAIN_1;

    return (sfe_ade7953_pga_gain_t)(value & 0x07);
}

bool sfDevADE7953::setDigitalGainIA(uint32_t gain)
{
    if (!_theBus)
        return false;

    return (_theBus->writeRegister(ksfADE7953RegAIGain, gain) == ksfTkErrOk);
}

uint32_t sfDevADE7953::getDigitalGainIA(void)
{
    uint32_t value = 0;

    if (!_theBus)
        return 0;

    if (_theBus->readRegister(ksfADE7953RegAIGain, value) != ksfTkErrOk)
        return 0;

    return value;
}

bool sfDevADE7953::setDigitalGainIB(uint32_t gain)
{
    if (!_theBus)
        return false;

    return (_theBus->writeRegister(ksfADE7953RegBIGain, gain) == ksfTkErrOk);
}

uint32_t sfDevADE7953::getDigitalGainIB(void)
{
    uint32_t value = 0;

    if (!_theBus)
        return 0;

    if (_theBus->readRegister(ksfADE7953RegBIGain, value) != ksfTkErrOk)
        return 0;

    return value;
}

// ======================= Current Measurement ==============================

uint32_t sfDevADE7953::getIRmsA(void)
{
    uint32_t value = 0;

    if (!_theBus)
        return 0;

    if (_theBus->readRegister(ksfADE7953RegIRmsA, value) != ksfTkErrOk)
        return 0;

    return value;
}

uint32_t sfDevADE7953::getIRmsB(void)
{
    uint32_t value = 0;

    if (!_theBus)
        return 0;

    if (_theBus->readRegister(ksfADE7953RegIRmsB, value) != ksfTkErrOk)
        return 0;

    return value;
}

int32_t sfDevADE7953::getInstantaneousIA(void)
{
    uint32_t raw = 0;

    if (!_theBus)
        return 0;

    if (_theBus->readRegister(ksfADE7953RegIA, raw) != ksfTkErrOk)
        return 0;

    // Sign-extend from 24-bit to 32-bit. The upper 8 bits from the 0x3xx read are
    // already sign-extended by the ADE7953, so we can cast directly.
    return (int32_t)raw;
}

int32_t sfDevADE7953::getInstantaneousIB(void)
{
    uint32_t raw = 0;

    if (!_theBus)
        return 0;

    if (_theBus->readRegister(ksfADE7953RegIB, raw) != ksfTkErrOk)
        return 0;

    return (int32_t)raw;
}

// ======================== Peak Detection ==================================

uint32_t sfDevADE7953::getPeakIA(void)
{
    uint32_t value = 0;

    if (!_theBus)
        return 0;

    if (_theBus->readRegister(ksfADE7953RegIAPeak, value) != ksfTkErrOk)
        return 0;

    return value;
}

uint32_t sfDevADE7953::getPeakIB(void)
{
    uint32_t value = 0;

    if (!_theBus)
        return 0;

    if (_theBus->readRegister(ksfADE7953RegIBPeak, value) != ksfTkErrOk)
        return 0;

    return value;
}

uint32_t sfDevADE7953::readAndResetPeakIA(void)
{
    uint32_t value = 0;

    if (!_theBus)
        return 0;

    // Reading RSTIAPEAK atomically reads the peak value and clears the register.
    if (_theBus->readRegister(ksfADE7953RegRstIAPeak, value) != ksfTkErrOk)
        return 0;

    return value;
}

uint32_t sfDevADE7953::readAndResetPeakIB(void)
{
    uint32_t value = 0;

    if (!_theBus)
        return 0;

    if (_theBus->readRegister(ksfADE7953RegRstIBPeak, value) != ksfTkErrOk)
        return 0;

    return value;
}

bool sfDevADE7953::setOvercurrentLevel(uint32_t level)
{
    if (!_theBus)
        return false;

    return (_theBus->writeRegister(ksfADE7953RegOiLvl, level) == ksfTkErrOk);
}

uint32_t sfDevADE7953::getOvercurrentLevel(void)
{
    uint32_t value = 0;

    if (!_theBus)
        return 0;

    if (_theBus->readRegister(ksfADE7953RegOiLvl, value) != ksfTkErrOk)
        return 0;

    return value;
}

// ====================== Calibration & Offset ==============================

bool sfDevADE7953::setIRmsOffsetA(int32_t offset)
{
    if (!_theBus)
        return false;

    return (_theBus->writeRegister(ksfADE7953RegAIRmsOS, (uint32_t)offset) == ksfTkErrOk);
}

int32_t sfDevADE7953::getIRmsOffsetA(void)
{
    uint32_t raw = 0;

    if (!_theBus)
        return 0;

    if (_theBus->readRegister(ksfADE7953RegAIRmsOS, raw) != ksfTkErrOk)
        return 0;

    return (int32_t)raw;
}

bool sfDevADE7953::setIRmsOffsetB(int32_t offset)
{
    if (!_theBus)
        return false;

    return (_theBus->writeRegister(ksfADE7953RegBIRmsOS, (uint32_t)offset) == ksfTkErrOk);
}

int32_t sfDevADE7953::getIRmsOffsetB(void)
{
    uint32_t raw = 0;

    if (!_theBus)
        return 0;

    if (_theBus->readRegister(ksfADE7953RegBIRmsOS, raw) != ksfTkErrOk)
        return 0;

    return (int32_t)raw;
}

// ======================= Zero-Crossing (ZX_I) =============================

bool sfDevADE7953::setZXISource(bool useChannelB)
{
    if (!_theBus)
        return false;

    uint16_t config = 0;
    if (_theBus->readRegister(ksfADE7953RegConfig, config) != ksfTkErrOk)
        return false;

    if (useChannelB)
        config |= ksfADE7953ConfigZXI;
    else
        config &= ~ksfADE7953ConfigZXI;

    return (_theBus->writeRegister(ksfADE7953RegConfig, config) == ksfTkErrOk);
}

bool sfDevADE7953::getZXISource(void)
{
    uint16_t config = 0;

    if (!_theBus)
        return false;

    if (_theBus->readRegister(ksfADE7953RegConfig, config) != ksfTkErrOk)
        return false;

    return (config & ksfADE7953ConfigZXI) != 0;
}

bool sfDevADE7953::setZXEdge(sfe_ade7953_zx_edge_t edge)
{
    if (!_theBus)
        return false;

    uint16_t config = 0;
    if (_theBus->readRegister(ksfADE7953RegConfig, config) != ksfTkErrOk)
        return false;

    // Clear the ZX_EDGE bits [13:12] and set the new value.
    config &= ~ksfADE7953ConfigZXEdgeMask;
    config |= ((uint16_t)edge << ksfADE7953ConfigZXEdgeShift) & ksfADE7953ConfigZXEdgeMask;

    return (_theBus->writeRegister(ksfADE7953RegConfig, config) == ksfTkErrOk);
}

sfe_ade7953_zx_edge_t sfDevADE7953::getZXEdge(void)
{
    uint16_t config = 0;

    if (!_theBus)
        return ADE7953_ZX_EDGE_BOTH;

    if (_theBus->readRegister(ksfADE7953RegConfig, config) != ksfTkErrOk)
        return ADE7953_ZX_EDGE_BOTH;

    return (sfe_ade7953_zx_edge_t)((config & ksfADE7953ConfigZXEdgeMask) >> ksfADE7953ConfigZXEdgeShift);
}

bool sfDevADE7953::enableZXLPF(bool enable)
{
    if (!_theBus)
        return false;

    uint16_t config = 0;
    if (_theBus->readRegister(ksfADE7953RegConfig, config) != ksfTkErrOk)
        return false;

    // Note: CONFIG bit 6 (ZXLPF) = 0 means LPF is ENABLED, 1 means DISABLED.
    // So we invert the user's intent.
    if (enable)
        config &= ~ksfADE7953ConfigZXLPF; // Clear bit to enable LPF
    else
        config |= ksfADE7953ConfigZXLPF; // Set bit to disable LPF

    return (_theBus->writeRegister(ksfADE7953RegConfig, config) == ksfTkErrOk);
}

bool sfDevADE7953::enableHPF(bool enable)
{
    if (!_theBus)
        return false;

    uint16_t config = 0;
    if (_theBus->readRegister(ksfADE7953RegConfig, config) != ksfTkErrOk)
        return false;

    if (enable)
        config |= ksfADE7953ConfigHPFEn;
    else
        config &= ~ksfADE7953ConfigHPFEn;

    return (_theBus->writeRegister(ksfADE7953RegConfig, config) == ksfTkErrOk);
}

bool sfDevADE7953::isHPFEnabled(void)
{
    if (!_theBus)
        return false;

    uint16_t config = 0;
    if (_theBus->readRegister(ksfADE7953RegConfig, config) != ksfTkErrOk)
        return false;

    return (config & ksfADE7953ConfigHPFEn) != 0;
}

uint16_t sfDevADE7953::getPeriod(void)
{
    uint16_t value = 0;

    if (!_theBus)
        return 0;

    if (_theBus->readRegister(ksfADE7953RegPeriod, value) != ksfTkErrOk)
        return 0;

    return value;
}

// =========================== Interrupts ===================================

bool sfDevADE7953::setInterruptEnableA(uint32_t mask)
{
    if (!_theBus)
        return false;

    return (_theBus->writeRegister(ksfADE7953RegIrqEnA, mask) == ksfTkErrOk);
}

uint32_t sfDevADE7953::getInterruptEnableA(void)
{
    uint32_t value = 0;

    if (!_theBus)
        return 0;

    if (_theBus->readRegister(ksfADE7953RegIrqEnA, value) != ksfTkErrOk)
        return 0;

    return value;
}

uint32_t sfDevADE7953::getInterruptStatusA(void)
{
    uint32_t value = 0;

    if (!_theBus)
        return 0;

    if (_theBus->readRegister(ksfADE7953RegIrqStatA, value) != ksfTkErrOk)
        return 0;

    return value;
}

uint32_t sfDevADE7953::readAndResetInterruptStatusA(void)
{
    uint32_t value = 0;

    if (!_theBus)
        return 0;

    // Reading RSTIRQSTATA atomically reads status and clears the flags.
    if (_theBus->readRegister(ksfADE7953RegRstIrqStatA, value) != ksfTkErrOk)
        return 0;

    return value;
}

bool sfDevADE7953::setInterruptEnableB(uint32_t mask)
{
    if (!_theBus)
        return false;

    return (_theBus->writeRegister(ksfADE7953RegIrqEnB, mask) == ksfTkErrOk);
}

uint32_t sfDevADE7953::getInterruptEnableB(void)
{
    uint32_t value = 0;

    if (!_theBus)
        return 0;

    if (_theBus->readRegister(ksfADE7953RegIrqEnB, value) != ksfTkErrOk)
        return 0;

    return value;
}

uint32_t sfDevADE7953::getInterruptStatusB(void)
{
    uint32_t value = 0;

    if (!_theBus)
        return 0;

    if (_theBus->readRegister(ksfADE7953RegIrqStatB, value) != ksfTkErrOk)
        return 0;

    return value;
}

uint32_t sfDevADE7953::readAndResetInterruptStatusB(void)
{
    uint32_t value = 0;

    if (!_theBus)
        return 0;

    if (_theBus->readRegister(ksfADE7953RegRstIrqStatB, value) != ksfTkErrOk)
        return 0;

    return value;
}

// ========================= No-Load Detection ==============================

bool sfDevADE7953::setNoLoadDisable(uint8_t mask)
{
    if (!_theBus)
        return false;

    return (_theBus->writeRegister(ksfADE7953RegDisNoLoad, (uint8_t)(mask & 0x07)) == ksfTkErrOk);
}

uint8_t sfDevADE7953::getNoLoadDisable(void)
{
    uint8_t value = 0;

    if (!_theBus)
        return 0;

    if (_theBus->readRegister(ksfADE7953RegDisNoLoad, value) != ksfTkErrOk)
        return 0;

    return value & 0x07;
}

// ====================== Line Cycle Accumulation ===========================

bool sfDevADE7953::setLineCycleMode(uint8_t mode)
{
    if (!_theBus)
        return false;

    return (_theBus->writeRegister(ksfADE7953RegLCycMode, mode) == ksfTkErrOk);
}

uint8_t sfDevADE7953::getLineCycleMode(void)
{
    uint8_t value = 0;

    if (!_theBus)
        return 0;

    if (_theBus->readRegister(ksfADE7953RegLCycMode, value) != ksfTkErrOk)
        return 0;

    return value;
}

bool sfDevADE7953::setLineCycleCount(uint16_t halfCycles)
{
    if (!_theBus)
        return false;

    return (_theBus->writeRegister(ksfADE7953RegLineCyc, halfCycles) == ksfTkErrOk);
}

uint16_t sfDevADE7953::getLineCycleCount(void)
{
    uint16_t value = 0;

    if (!_theBus)
        return 0;

    if (_theBus->readRegister(ksfADE7953RegLineCyc, value) != ksfTkErrOk)
        return 0;

    return value;
}

// ========================= Sag Detection ==================================

bool sfDevADE7953::setSagCycles(uint8_t cycles)
{
    if (!_theBus)
        return false;

    return (_theBus->writeRegister(ksfADE7953RegSagCyc, cycles) == ksfTkErrOk);
}

uint8_t sfDevADE7953::getSagCycles(void)
{
    uint8_t value = 0;

    if (!_theBus)
        return 0;

    if (_theBus->readRegister(ksfADE7953RegSagCyc, value) != ksfTkErrOk)
        return 0;

    return value;
}

bool sfDevADE7953::setSagLevel(uint32_t level)
{
    if (!_theBus)
        return false;

    return (_theBus->writeRegister(ksfADE7953RegSagLvl, level) == ksfTkErrOk);
}

uint32_t sfDevADE7953::getSagLevel(void)
{
    uint32_t value = 0;

    if (!_theBus)
        return 0;

    if (_theBus->readRegister(ksfADE7953RegSagLvl, value) != ksfTkErrOk)
        return 0;

    return value;
}

// ========================== Diagnostics ===================================

uint8_t sfDevADE7953::getLastOperation(void)
{
    uint8_t value = 0;

    if (!_theBus)
        return 0;

    if (_theBus->readRegister(ksfADE7953RegLastOp, value) != ksfTkErrOk)
        return 0;

    return value;
}

uint16_t sfDevADE7953::getLastAddress(void)
{
    uint16_t value = 0;

    if (!_theBus)
        return 0;

    if (_theBus->readRegister(ksfADE7953RegLastAdd, value) != ksfTkErrOk)
        return 0;

    return value;
}

uint8_t sfDevADE7953::getLastData8(void)
{
    uint8_t value = 0;

    if (!_theBus)
        return 0;

    if (_theBus->readRegister(ksfADE7953RegLastRwData8, value) != ksfTkErrOk)
        return 0;

    return value;
}

uint16_t sfDevADE7953::getLastData16(void)
{
    uint16_t value = 0;

    if (!_theBus)
        return 0;

    if (_theBus->readRegister(ksfADE7953RegLastRwData16, value) != ksfTkErrOk)
        return 0;

    return value;
}

uint32_t sfDevADE7953::getLastData32(void)
{
    uint32_t value = 0;

    if (!_theBus)
        return 0;

    if (_theBus->readRegister(ksfADE7953RegLastRwData32, value) != ksfTkErrOk)
        return 0;

    return value;
}