/**
 * ASCOMAlpaca.h - ASCOM Alpaca REST API Interface
 *
 * Implements ASCOM Alpaca protocol for Cover Calibrator device
 * Specification: https://ascom-standards.org/api/
 */

#ifndef ASCOM_ALPACA_H
#define ASCOM_ALPACA_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include "CoverCalibrator.h"
#include "Config.h"

// ============================================================================
// ASCOM ALPACA API CLASS
// ============================================================================

class ASCOMAlpaca {
public:
    // Constructor
    ASCOMAlpaca(CoverCalibrator& device, WebServer& server);

    // Initialization
    void begin();

private:
    // ========================================================================
    // HTTP ENDPOINT HANDLERS
    // ========================================================================

    // Management API endpoints
    void handleManagementVersions();
    void handleManagementDescription();
    void handleManagementConfiguredDevices();

    // Common API endpoints
    void handleAction();
    void handleCommandBlind();
    void handleCommandBool();
    void handleCommandString();
    void handleConnected();
    void handleDescription();
    void handleDriverInfo();
    void handleDriverVersion();
    void handleInterfaceVersion();
    void handleName();
    void handleSupportedActions();

    // CoverCalibrator-specific endpoints
    void handleBrightness();
    void handleCalibratorState();
    void handleCoverState();
    void handleMaxBrightness();
    void handleCalibratorOff();
    void handleCalibratorOn();
    void handleCloseCover();
    void handleHaltCover();
    void handleOpenCover();

    // ========================================================================
    // UTILITY METHODS
    // ========================================================================

    // Response builders
    void sendJsonResponse(int errorNumber, const String& errorMessage);
    void sendJsonResponse(int errorNumber, const String& errorMessage, const String& key, int value);
    void sendJsonResponse(int errorNumber, const String& errorMessage, const String& key, const String& value);
    void sendJsonResponse(int errorNumber, const String& errorMessage, const String& key, bool value);
    void sendErrorResponse(int errorCode, const String& errorMessage);
    void sendSuccessResponse();

    // Parameter extraction
    bool getClientInfo(uint32_t& clientID, uint32_t& clientTransactionID);
    bool getIntParam(const String& name, int& value);
    bool getBoolParam(const String& name, bool& value);

    // Error codes (ASCOM standard)
    enum ASCOMErrorCode {
        ERROR_OK = 0,
        ERROR_NOT_IMPLEMENTED = 0x400,
        ERROR_INVALID_VALUE = 0x401,
        ERROR_VALUE_NOT_SET = 0x402,
        ERROR_NOT_CONNECTED = 0x407,
        ERROR_INVALID_OPERATION = 0x40B,
        ERROR_UNSPECIFIED = 0xFFFF
    };

    // ========================================================================
    // MEMBER VARIABLES
    // ========================================================================

    CoverCalibrator& coverCalibrator;
    WebServer& server;
    uint32_t serverTransactionID;
};

#endif // ASCOM_ALPACA_H
