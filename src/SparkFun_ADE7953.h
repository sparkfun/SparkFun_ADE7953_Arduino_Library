/**
 * @file SparkFun_ADE7953.h
 * @brief Arduino-specific implementation for the SparkFun ADE7953 Energy Meter.
 *
 * @details
 * This file provides the Arduino-specific wrapper for the ADE7953 driver class.
 * The SfeADE7953ArdI2C class inherits from the platform-independent sfDevADE7953
 * driver and implements I2C communication using Arduino's Wire library via the
 * SparkFun Toolkit. See the examples folder for usage.
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

// clang-format off
#include <SparkFun_Toolkit.h>
#include "sfTk/sfDevADE7953.h"
#include <Arduino.h>
// clang-format on

/**
 * @class SfeADE7953ArdI2C
 * @brief Arduino I2C implementation for the ADE7953 energy meter.
 *
 * @details
 * This class provides Arduino-specific I2C communication for the ADE7953.
 * It inherits all register access methods from sfDevADE7953 and adds the
 * begin() method required for Arduino initialization. The class owns an
 * sfTkArdI2C bus object which wraps the Arduino Wire library.
 *
 * @see sfDevADE7953
 * @see TwoWire
 */
class SfeADE7953ArdI2C : public sfDevADE7953
{
  public:
    SfeADE7953ArdI2C()
    {
    }

    /**
     * @brief Initializes the ADE7953 with I2C communication.
     *
     * @details
     * Initializes the Toolkit I2C bus, confirms the device is present on the bus, then calls the
     * base class begin() to verify the device identity, configure byte order, apply the optimized
     * performance register sequence, and set default gain / HPF configuration.
     *
     * @param address 7-bit I2C address of the device (the ADE7953 uses a single fixed address).
     * @param wirePort TwoWire instance to use for I2C communication (default: Wire).
     *
     * @return true If initialization is successful.
     * @return false If any initialization step fails.
     */
    bool begin(uint8_t address = kI2CAddress, TwoWire &wirePort = Wire)
    {
        // Initialize the Toolkit I2C bus with the given Wire port and address.
        if (_theI2CBus.init(wirePort, address) != ksfTkErrOk)
            return false;

        // Confirm a device is actually responding at this address before we configure anything.
        if (_theI2CBus.ping() != ksfTkErrOk)
            return false;

        // The base class begin() verifies the device identity, sets byte order, writes the
        // optimized performance register, and applies default configuration.
        return sfDevADE7953::begin(&_theI2CBus) == ksfTkErrOk;
    }

  private:
    /// @brief Arduino I2C bus interface instance used for all communication with the ADE7953.
    sfTkArdI2C _theI2CBus;
};
