/**
 * ASCOMAlpaca.cpp - Implementation of ASCOM Alpaca REST API
 */

#include "ASCOMAlpaca.h"

// ============================================================================
// CONSTRUCTOR
// ============================================================================

ASCOMAlpaca::ASCOMAlpaca(CoverCalibrator& device)
    : coverCalibrator(device),
      server(ALPACA_PORT),
      serverTransactionID(0)
{
}

// ============================================================================
// INITIALIZATION
// ============================================================================

void ASCOMAlpaca::begin() {
    DEBUG_PRINTLN("Starting ASCOM Alpaca interface...");

    // Connect to WiFi
    DEBUG_PRINTF("Connecting to WiFi SSID: %s\n", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        DEBUG_PRINT(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        DEBUG_PRINTLN("");
        DEBUG_PRINTF("WiFi connected! IP: %s\n", WiFi.localIP().toString().c_str());
    } else {
        DEBUG_PRINTLN("");
        DEBUG_PRINTLN("WiFi connection failed!");
        return;
    }

    // Setup Management API endpoints
    server.on("/management/apiversions", HTTP_GET, [this]() { handleManagementVersions(); });
    server.on("/management/v1/description", HTTP_GET, [this]() { handleManagementDescription(); });
    server.on("/management/v1/configureddevices", HTTP_GET, [this]() { handleManagementConfiguredDevices(); });

    // Setup Common API endpoints
    String basePath = "/api/v1/covercalibrator/" + String(DEVICE_NUMBER) + "/";

    server.on((basePath + "action").c_str(), HTTP_PUT, [this]() { handleAction(); });
    server.on((basePath + "commandblind").c_str(), HTTP_PUT, [this]() { handleCommandBlind(); });
    server.on((basePath + "commandbool").c_str(), HTTP_PUT, [this]() { handleCommandBool(); });
    server.on((basePath + "commandstring").c_str(), HTTP_PUT, [this]() { handleCommandString(); });
    server.on((basePath + "connected").c_str(), HTTP_GET, [this]() { handleConnected(); });
    server.on((basePath + "connected").c_str(), HTTP_PUT, [this]() { handleConnected(); });
    server.on((basePath + "description").c_str(), HTTP_GET, [this]() { handleDescription(); });
    server.on((basePath + "driverinfo").c_str(), HTTP_GET, [this]() { handleDriverInfo(); });
    server.on((basePath + "driverversion").c_str(), HTTP_GET, [this]() { handleDriverVersion(); });
    server.on((basePath + "interfaceversion").c_str(), HTTP_GET, [this]() { handleInterfaceVersion(); });
    server.on((basePath + "name").c_str(), HTTP_GET, [this]() { handleName(); });
    server.on((basePath + "supportedactions").c_str(), HTTP_GET, [this]() { handleSupportedActions(); });

    // Setup CoverCalibrator-specific endpoints
    server.on((basePath + "brightness").c_str(), HTTP_GET, [this]() { handleBrightness(); });
    server.on((basePath + "calibratorstate").c_str(), HTTP_GET, [this]() { handleCalibratorState(); });
    server.on((basePath + "coverstate").c_str(), HTTP_GET, [this]() { handleCoverState(); });
    server.on((basePath + "maxbrightness").c_str(), HTTP_GET, [this]() { handleMaxBrightness(); });
    server.on((basePath + "calibratoroff").c_str(), HTTP_PUT, [this]() { handleCalibratorOff(); });
    server.on((basePath + "calibratoron").c_str(), HTTP_PUT, [this]() { handleCalibratorOn(); });
    server.on((basePath + "closecover").c_str(), HTTP_PUT, [this]() { handleCloseCover(); });
    server.on((basePath + "haltcover").c_str(), HTTP_PUT, [this]() { handleHaltCover(); });
    server.on((basePath + "opencover").c_str(), HTTP_PUT, [this]() { handleOpenCover(); });

    // Start server
    server.begin();
    DEBUG_PRINTF("ASCOM Alpaca server started on port %d\n", ALPACA_PORT);
}

// ============================================================================
// UPDATE
// ============================================================================

void ASCOMAlpaca::update() {
    server.handleClient();
}

// ============================================================================
// MANAGEMENT API ENDPOINTS
// ============================================================================

void ASCOMAlpaca::handleManagementVersions() {
    String json = "{\"Value\":[1],\"ErrorNumber\":0,\"ErrorMessage\":\"\"}";
    server.send(200, "application/json", json);
}

void ASCOMAlpaca::handleManagementDescription() {
    StaticJsonDocument<512> doc;
    doc["ServerName"] = DEVICE_NAME;
    doc["Manufacturer"] = MANUFACTURER;
    doc["ManufacturerVersion"] = MANUFACTURER_VERSION;
    doc["Location"] = "ESP32";

    String json;
    serializeJson(doc, json);
    server.send(200, "application/json", json);
}

void ASCOMAlpaca::handleManagementConfiguredDevices() {
    StaticJsonDocument<1024> doc;
    JsonArray devices = doc.createNestedArray("Value");

    JsonObject device = devices.createNestedObject();
    device["DeviceName"] = DEVICE_NAME;
    device["DeviceType"] = DEVICE_TYPE;
    device["DeviceNumber"] = DEVICE_NUMBER;
    device["UniqueID"] = UNIQUE_ID;

    doc["ErrorNumber"] = 0;
    doc["ErrorMessage"] = "";

    String json;
    serializeJson(doc, json);
    server.send(200, "application/json", json);
}

// ============================================================================
// COMMON API ENDPOINTS
// ============================================================================

void ASCOMAlpaca::handleAction() {
    sendErrorResponse(ERROR_NOT_IMPLEMENTED, "Action not implemented");
}

void ASCOMAlpaca::handleCommandBlind() {
    sendErrorResponse(ERROR_NOT_IMPLEMENTED, "CommandBlind not implemented");
}

void ASCOMAlpaca::handleCommandBool() {
    sendErrorResponse(ERROR_NOT_IMPLEMENTED, "CommandBool not implemented");
}

void ASCOMAlpaca::handleCommandString() {
    sendErrorResponse(ERROR_NOT_IMPLEMENTED, "CommandString not implemented");
}

void ASCOMAlpaca::handleConnected() {
    if (server.method() == HTTP_GET) {
        // GET - return connection state
        sendJsonResponse(ERROR_OK, "", "Value", coverCalibrator.isConnected());
    } else {
        // PUT - set connection state
        bool connected;
        if (getBoolParam("Connected", connected)) {
            coverCalibrator.setConnected(connected);
            sendSuccessResponse();
        } else {
            sendErrorResponse(ERROR_INVALID_VALUE, "Invalid Connected parameter");
        }
    }
}

void ASCOMAlpaca::handleDescription() {
    sendJsonResponse(ERROR_OK, "", "Value", coverCalibrator.getDescription());
}

void ASCOMAlpaca::handleDriverInfo() {
    sendJsonResponse(ERROR_OK, "", "Value", coverCalibrator.getDriverInfo());
}

void ASCOMAlpaca::handleDriverVersion() {
    sendJsonResponse(ERROR_OK, "", "Value", coverCalibrator.getDriverVersion());
}

void ASCOMAlpaca::handleInterfaceVersion() {
    sendJsonResponse(ERROR_OK, "", "Value", 1);  // CoverCalibrator interface version is 1
}

void ASCOMAlpaca::handleName() {
    sendJsonResponse(ERROR_OK, "", "Value", coverCalibrator.getName());
}

void ASCOMAlpaca::handleSupportedActions() {
    // Return empty array - no custom actions supported
    StaticJsonDocument<256> doc;
    JsonArray actions = doc.createNestedArray("Value");
    doc["ErrorNumber"] = 0;
    doc["ErrorMessage"] = "";

    String json;
    serializeJson(doc, json);
    server.send(200, "application/json", json);
}

// ============================================================================
// COVERCALIBRATOR-SPECIFIC ENDPOINTS
// ============================================================================

void ASCOMAlpaca::handleBrightness() {
    if (!coverCalibrator.isConnected()) {
        sendErrorResponse(ERROR_NOT_CONNECTED, "Device not connected");
        return;
    }
    sendJsonResponse(ERROR_OK, "", "Value", coverCalibrator.getBrightness());
}

void ASCOMAlpaca::handleCalibratorState() {
    if (!coverCalibrator.isConnected()) {
        sendErrorResponse(ERROR_NOT_CONNECTED, "Device not connected");
        return;
    }
    sendJsonResponse(ERROR_OK, "", "Value", (int)coverCalibrator.getCalibratorState());
}

void ASCOMAlpaca::handleCoverState() {
    if (!coverCalibrator.isConnected()) {
        sendErrorResponse(ERROR_NOT_CONNECTED, "Device not connected");
        return;
    }
    sendJsonResponse(ERROR_OK, "", "Value", (int)coverCalibrator.getCoverState());
}

void ASCOMAlpaca::handleMaxBrightness() {
    if (!coverCalibrator.isConnected()) {
        sendErrorResponse(ERROR_NOT_CONNECTED, "Device not connected");
        return;
    }
    sendJsonResponse(ERROR_OK, "", "Value", coverCalibrator.getMaxBrightness());
}

void ASCOMAlpaca::handleCalibratorOff() {
    if (!coverCalibrator.isConnected()) {
        sendErrorResponse(ERROR_NOT_CONNECTED, "Device not connected");
        return;
    }

    coverCalibrator.calibratorOff();
    sendSuccessResponse();
}

void ASCOMAlpaca::handleCalibratorOn() {
    if (!coverCalibrator.isConnected()) {
        sendErrorResponse(ERROR_NOT_CONNECTED, "Device not connected");
        return;
    }

    int brightness;
    if (getIntParam("Brightness", brightness)) {
        coverCalibrator.calibratorOn(brightness);
        sendSuccessResponse();
    } else {
        sendErrorResponse(ERROR_INVALID_VALUE, "Invalid Brightness parameter");
    }
}

void ASCOMAlpaca::handleCloseCover() {
    if (!coverCalibrator.isConnected()) {
        sendErrorResponse(ERROR_NOT_CONNECTED, "Device not connected");
        return;
    }

    coverCalibrator.closeCover();
    sendSuccessResponse();
}

void ASCOMAlpaca::handleHaltCover() {
    if (!coverCalibrator.isConnected()) {
        sendErrorResponse(ERROR_NOT_CONNECTED, "Device not connected");
        return;
    }

    coverCalibrator.haltCover();
    sendSuccessResponse();
}

void ASCOMAlpaca::handleOpenCover() {
    if (!coverCalibrator.isConnected()) {
        sendErrorResponse(ERROR_NOT_CONNECTED, "Device not connected");
        return;
    }

    coverCalibrator.openCover();
    sendSuccessResponse();
}

// ============================================================================
// UTILITY METHODS
// ============================================================================

void ASCOMAlpaca::sendJsonResponse(int errorNumber, const String& errorMessage) {
    StaticJsonDocument<256> doc;
    doc["ErrorNumber"] = errorNumber;
    doc["ErrorMessage"] = errorMessage;
    doc["ServerTransactionID"] = serverTransactionID++;

    uint32_t clientID, clientTransactionID;
    if (getClientInfo(clientID, clientTransactionID)) {
        doc["ClientTransactionID"] = clientTransactionID;
        doc["ClientID"] = clientID;
    }

    String json;
    serializeJson(doc, json);
    server.send(200, "application/json", json);
}

void ASCOMAlpaca::sendJsonResponse(int errorNumber, const String& errorMessage,
                                   const String& key, int value) {
    StaticJsonDocument<256> doc;
    doc[key] = value;
    doc["ErrorNumber"] = errorNumber;
    doc["ErrorMessage"] = errorMessage;
    doc["ServerTransactionID"] = serverTransactionID++;

    uint32_t clientID, clientTransactionID;
    if (getClientInfo(clientID, clientTransactionID)) {
        doc["ClientTransactionID"] = clientTransactionID;
        doc["ClientID"] = clientID;
    }

    String json;
    serializeJson(doc, json);
    server.send(200, "application/json", json);
}

void ASCOMAlpaca::sendJsonResponse(int errorNumber, const String& errorMessage,
                                   const String& key, const String& value) {
    StaticJsonDocument<512> doc;
    doc[key] = value;
    doc["ErrorNumber"] = errorNumber;
    doc["ErrorMessage"] = errorMessage;
    doc["ServerTransactionID"] = serverTransactionID++;

    uint32_t clientID, clientTransactionID;
    if (getClientInfo(clientID, clientTransactionID)) {
        doc["ClientTransactionID"] = clientTransactionID;
        doc["ClientID"] = clientID;
    }

    String json;
    serializeJson(doc, json);
    server.send(200, "application/json", json);
}

void ASCOMAlpaca::sendJsonResponse(int errorNumber, const String& errorMessage,
                                   const String& key, bool value) {
    StaticJsonDocument<256> doc;
    doc[key] = value;
    doc["ErrorNumber"] = errorNumber;
    doc["ErrorMessage"] = errorMessage;
    doc["ServerTransactionID"] = serverTransactionID++;

    uint32_t clientID, clientTransactionID;
    if (getClientInfo(clientID, clientTransactionID)) {
        doc["ClientTransactionID"] = clientTransactionID;
        doc["ClientID"] = clientID;
    }

    String json;
    serializeJson(doc, json);
    server.send(200, "application/json", json);
}

void ASCOMAlpaca::sendErrorResponse(int errorCode, const String& errorMessage) {
    sendJsonResponse(errorCode, errorMessage);
}

void ASCOMAlpaca::sendSuccessResponse() {
    sendJsonResponse(ERROR_OK, "");
}

bool ASCOMAlpaca::getClientInfo(uint32_t& clientID, uint32_t& clientTransactionID) {
    bool hasClientID = false;
    bool hasClientTransactionID = false;

    for (int i = 0; i < server.args(); i++) {
        String argName = server.argName(i);
        argName.toLowerCase();

        if (argName == "clientid") {
            clientID = server.arg(i).toInt();
            hasClientID = true;
        } else if (argName == "clienttransactionid") {
            clientTransactionID = server.arg(i).toInt();
            hasClientTransactionID = true;
        }
    }

    return hasClientID && hasClientTransactionID;
}

bool ASCOMAlpaca::getIntParam(const String& name, int& value) {
    if (server.hasArg(name)) {
        value = server.arg(name).toInt();
        return true;
    }
    return false;
}

bool ASCOMAlpaca::getBoolParam(const String& name, bool& value) {
    if (server.hasArg(name)) {
        String arg = server.arg(name);
        arg.toLowerCase();
        value = (arg == "true" || arg == "1");
        return true;
    }
    return false;
}
