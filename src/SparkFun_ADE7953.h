/**
 * @file SparkFun_ADE7953.h
 * @brief Arduino-specific implementation for the SparkFun ADE7953 Energy Meter.
 *
 * @details
 * This file provides the Arduino-specific wrapper for the ADE7953 driver class.
 * The SfeADE7953ArdI2C class inherits from the platform-independent sfDevADE7953
 * driver and implements I2C communication using Arduino's Wire library via the
 * SparkFun Toolkit.
 *
 * Typical usage:
 * @code
 * #include <SparkFun_ADE7953.h>
 *
 * SfeADE7953ArdI2C ammeter;
 *
 * void setup() {
 *     Wire.begin();
 *     if (!ammeter.begin()) {
 *         Serial.println("ADE7953 not found!");
 *         while (1);
 *     }
 *     ammeter.setGainIA(ADE7953_PGA_GAIN_4); // 4x gain for 5.6 ohm shunt
 * }
 *
 * void loop() {
 *     uint32_t irms = ammeter.getIRmsA();
 *     Serial.println(irms);
 *     delay(500);
 * }
 * @endcode
 *
 * @author SparkFun Electronics
 * @date 2025
 * @copyright Copyright (c) 2025, SparkFun Electronics Inc. All rights reserved.
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
 * begin() and isConnected() methods required for Arduino initialization.
 *
 * The class owns an sfTkArdI2C bus object which wraps the Arduino Wire library.
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
     * This method performs the following initialization steps:
     * 1. Initializes the Toolkit I2C bus with the given address and Wire port
     * 2. Calls the base class begin() to set the bus, configure byte order,
     *    and write the optimized performance register
     * 3. Verifies the device is connected via ping and identity check
     *
     * @param address 7-bit I2C address of the device (default: 0x38)
     * @param wirePort TwoWire instance to use for I2C communication (default: Wire)
     *
     * @return true If initialization is successful
     * @return false If any initialization step fails
     *
     * Example:
     * @code
     * SfeADE7953ArdI2C ammeter;
     * Wire.begin();
     * if (!ammeter.begin()) {
     *     Serial.println("ADE7953 initialization failed!");
     *     while (1);
     * }
     * @endcode
     */
    bool begin(const uint8_t &address = kADE7953DefaultAddr, TwoWire &wirePort = Wire)
    {
        // Initialize the Toolkit I2C bus with the given Wire port and address.
        if (_theI2CBus.init(wirePort, address) != ksfTkErrOk)
            return false;

        // Call the base class begin() which sets the bus pointer,
        // configures byte order, and writes the optimized performance register.
        if (!sfDevADE7953::begin(&_theI2CBus))
            return false;

        // Verify the device is present and responding correctly.
        return isConnected();
    }

    /**
     * @brief Checks if the ADE7953 is connected and responding.
     *
     * @details
     * Pings the device at the configured I2C address and verifies identity
     * by reading the silicon version register to confirm an ADE7953 is present.
     *
     * @return true If the device responds and identity is verified
     * @return false If communication fails or identity check fails
     *
     * Example:
     * @code
     * if (!ammeter.isConnected()) {
     *     Serial.println("ADE7953 not found on bus!");
     * }
     * @endcode
     */
    bool isConnected(void)
    {
        // Ping the device at the configured I2C address.
        if (_theI2CBus.ping() != ksfTkErrOk)
            return false;

        // Verify we are talking to an ADE7953 by reading the version register.
        uint8_t version = getVersion();
        return (version != 0);
    }

  private:
    /**
     * @brief Arduino I2C bus interface instance.
     *
     * @details
     * This member handles low-level I2C communication between the Arduino and the ADE7953.
     * It is configured during begin() and used by all communication methods inherited from
     * sfDevADE7953.
     *
     * @see sfTkArdI2C
     * @see begin()
     */
    sfTkArdI2C _theI2CBus;
};