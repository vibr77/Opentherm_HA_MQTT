
/*
__   _____ ___ ___        Author: Vincent BESSON
 \ \ / /_ _| _ ) _ \      Release: 0.34
  \ V / | || _ \   /      Date: 20230709
   \_/ |___|___/_|_\      Description: ESP32 Mini Home Assistant Opentherm Master Thermostat
                2023      Licence: Creative Commons
______________________
*/ 
#include <Arduino.h>
#include "pwd.h"
#pragma once

#ifndef _CONFIG_
#define _CONFIG_

static const char * SW_VERSION="0.40";

// MOVE TO pwd.h

//static const char* WIFI_SSID="";
//static const char* WIFI_KEY="";

//static const char* MQTT_SERVER = "";
//static const char* MQTT_USER = "";
//static const char* MQTT_PASS = "";

static const int   MQTT_PORT = 1883;

static const char * MQTT_DEVICENAME="opentherm-Boiler";
static const String MQTT_DEV_UNIQUE_ID="OT_B_01";

static const int HTTP_PORT = 80;

// Master OpenTherm Shield pins configuration
static const int OT_IN_PIN = 21;  //for Arduino, 4 for ESP8266 (D2), 21 for ESP32
static const int OT_OUT_PIN = 22; //for Arduino, 5 for ESP8266 (D1), 22 for ESP32

// Temperature sensor pin
static const int ROOM_TEMP_SENSOR_PIN = 14; //for Arduino, 14 for ESP8266 (D5), 18 for ESP32

// Default Value

static const char * TEMPERATURE_UNIT= "C";
static const float MIN_SP_TEMP=5.0;
static const float MAX_SP_TEMP=30.0;
static const float INITIAL_TEMP=19.0;
static const float PRECISION=0.5;
static const float TEMPERATURE_STEP=0.5;
static  const boolean OPTIMISTIC=false;

static const float LOW_BAND_TEMP=20.0;
static const float HIGH_BAND_TEMP=65;
static const float NO_SP_TEMP_OVERRIDE=25;
static const int MAX_MODULATION_LEVEL=100;

static const String BASE_TOPIC = "homeassistant/opentherm-thermostat/";

static const String CLIMATE_BASE_TOPIC = "homeassistant/climate/opentherm-thermostat/";
static const String DISCOVERY_CLIMATE_TOPIC = CLIMATE_BASE_TOPIC + "config";

static const String FLAME_BASE_TOPIC = "homeassistant/binary_sensor/opentherm-thermostat/flame/";
static const String DISCOVERY_FLAME_TOPIC = FLAME_BASE_TOPIC + "config";

static const String FLAME_LEVEL_BASE_TOPIC = "homeassistant/sensor/opentherm-thermostat/flame_level/";
static const String DISCOVERY_FLAME_LEVEL_TOPIC = FLAME_LEVEL_BASE_TOPIC + "config";


static const String WATER_HEATING_BASE_TOPIC = "homeassistant/binary_sensor/opentherm-thermostat/waterheating/";
static const String DISCOVERY_WATER_HEATING_TOPIC = WATER_HEATING_BASE_TOPIC + "config";

static const String CENTRAL_HEATING_BASE_TOPIC = "homeassistant/binary_sensor/opentherm-thermostat/centralheating/";
static const String DISCOVERY_CENTRAL_HEATING_TOPIC = CENTRAL_HEATING_BASE_TOPIC + "config";

static const String BOILERTEMP_BASE_TOPIC = "homeassistant/sensor/opentherm-thermostat/boilertemp/";
static const String DISCOVERY_BOILER_TEMP_TOPIC = BOILERTEMP_BASE_TOPIC + "config";

static const String BOILER_RETURN_TEMP_BASE_TOPIC = "homeassistant/sensor/opentherm-thermostat/boiler_return_temp/";
static const String DISCOVERY_BOILER_RETURN_TEMP_TOPIC = BOILER_RETURN_TEMP_BASE_TOPIC + "config";

static const String BOILER_TARGET_TEMP_BASE_TOPIC = "homeassistant/sensor/opentherm-thermostat/boiler_target_temp/";
static const String DISCOVERY_BOILER_TARGET_TEMP_TOPIC = BOILER_TARGET_TEMP_BASE_TOPIC + "config";

static const String INTEGRAL_ERROR_BASE_TOPIC = "homeassistant/sensor/opentherm-thermostat/integral_error/";
static const String DISCOVERY_INTEGRAL_ERROR_TOPIC = INTEGRAL_ERROR_BASE_TOPIC + "config";

static const String DWHTEMP_BASE_TOPIC = "homeassistant/sensor/opentherm-thermostat/dwhtemp/";
static const String DISCOVERY_DWH_TEMP_TOPIC = DWHTEMP_BASE_TOPIC + "config";


static const String MAX_MODLVL_BASE_TOPIC = "homeassistant/number/opentherm-thermostat/maxmodulationlevel/";
static const String DISCOVERY_MAX_MODLVL_TOPIC = MAX_MODLVL_BASE_TOPIC + "config";

static const String HBAND_TEMP_BASE_TOPIC = "homeassistant/number/opentherm-thermostat/high_band_temperature/";
static const String DISCOVERY_HBAND_TEMP_TOPIC = HBAND_TEMP_BASE_TOPIC + "config";

static const String LBAND_TEMP_BASE_TOPIC = "homeassistant/number/opentherm-thermostat/low_band_temperature/";
static const String DISCOVERY_LBAND_TEMP_TOPIC = LBAND_TEMP_BASE_TOPIC + "config";

static const String NOSP_OVERRIDE_TEMP_BASE_TOPIC = "homeassistant/number/opentherm-thermostat/no_sp_temp_override/";
static const String DISCOVERY_NOSP_OVERRIDE_TEMP_TOPIC = NOSP_OVERRIDE_TEMP_BASE_TOPIC + "config";

static const String DISCOVERY_TEMP_DHW_BASE_TOPIC = "homeassistant/number/opentherm-thermostat/dwh_temp/";
static const String DISCOVERY_TEMP_DHW_TOPIC  = DISCOVERY_TEMP_DHW_BASE_TOPIC + "config";


static const String ENABLE_CHEATING_BASE_TOPIC = "homeassistant/switch/opentherm-thermostat/enable_cheating/";
static const String DISCOVERY_ENABLE_CHEATING_TOPIC = ENABLE_CHEATING_BASE_TOPIC + "config";

static const String ENABLE_WHEATING_BASE_TOPIC = "homeassistant/switch/opentherm-thermostat/enable_wheating/";
static const String DISCOVERY_ENABLE_WHEATING_TOPIC = ENABLE_WHEATING_BASE_TOPIC + "config";

static const String ENABLE_OT_LOG_BASE_TOPIC = "homeassistant/switch/opentherm-thermostat/enable_ot_log/";
static const String DISCOVERY_ENABLE_OT_LOG_TOPIC = ENABLE_OT_LOG_BASE_TOPIC + "config";

static const String OT_LOG_BASE_TOPIC = "homeassistant/sensor/opentherm-thermostat/ot_log/";
static const String DISCOVERY_OT_LOG_TOPIC = OT_LOG_BASE_TOPIC + "config";

static const String WIFI_SSID_BASE_TOPIC = "homeassistant/sensor/opentherm-thermostat/wifi_ssid/";
static const String DISCOVERY_WIFI_SSID_TOPIC = WIFI_SSID_BASE_TOPIC + "config";

static const String WIFI_RSSI_BASE_TOPIC = "homeassistant/sensor/opentherm-thermostat/wifi_rssi/";
static const String DISCOVERY_WIFI_RSSI_TOPIC = WIFI_RSSI_BASE_TOPIC + "config";

static const String IP_ADDR_BASE_TOPIC = "homeassistant/sensor/opentherm-thermostat/ip_addr/";
static const String DISCOVERY_IP_ADDR_TOPIC = IP_ADDR_BASE_TOPIC + "config";

static const String MAC_ADDR_BASE_TOPIC = "homeassistant/sensor/opentherm-thermostat/mac_addr/";
static const String DISCOVERY_MAC_ADDR_TOPIC = MAC_ADDR_BASE_TOPIC + "config";

static const String PING_ALIVE_BASE_TOPIC = "homeassistant/sensor/opentherm-thermostat/ping_alive/";
static const String DISCOVERY_PING_ALIVE_TOPIC = PING_ALIVE_BASE_TOPIC + "config";

const String AVAILABILITY_TOPIC = BASE_TOPIC + "status";

const String CURRENT_TEMP_STATE_TOPIC = BASE_TOPIC + "current-temperature/state";
const String CURRENT_TEMP_SET_TOPIC = BASE_TOPIC + "current-temperature/set";

// current temperature topics
const String TEMP_SETPOINT_STATE_TOPIC = BASE_TOPIC + "setpoint-temperature/state";
const String TEMP_SETPOINT_SET_TOPIC = BASE_TOPIC + "setpoint-temperature/set";

// working mode topics
const String MODE_STATE_TOPIC = BASE_TOPIC + "mode/state";
const String MODE_SET_TOPIC = BASE_TOPIC + "mode/set";

// Centralheating
const String CENTRAL_HEATING_STATE_TOPIC = BASE_TOPIC + "centralheating/state";

// Waterheating
const String WATER_HEATING_STATE_TOPIC = BASE_TOPIC + "waterheating/state";
// Enable Central Heating
const String ENABLE_CHEATING_STATE_TOPIC = BASE_TOPIC + "enable_cheating/state";
const String ENABLE_CHEATING_SET_TOPIC = BASE_TOPIC + "enable_cheating/set";

// Enable Water Heating
const String ENABLE_WHEATING_STATE_TOPIC = BASE_TOPIC + "enable_wheating/state";
const String ENABLE_WHEATING_SET_TOPIC = BASE_TOPIC + "enable_wheating/set";

// Enable OT Log
const String ENABLE_OT_LOG_STATE_TOPIC = BASE_TOPIC + "enable_ot_log/state";
const String ENABLE_OT_LOG_SET_TOPIC = BASE_TOPIC + "enable_ot_log/set";

// OT Log
const String OT_LOG_STATE_TOPIC = BASE_TOPIC + "ot_log/state";
//const String OT_LOG_SET_TOPIC = BASE_TOPIC + "ot_log/set";

// Modulation Level

const String MAX_MODULATION_LEVEL_STATE_TOPIC = BASE_TOPIC + "max_modulation_level/state";
const String MAX_MODULATION_LEVEL_SET_TOPIC = BASE_TOPIC + "max_modulation_level/set";


const String LBAND_TEMP_STATE_TOPIC = BASE_TOPIC + "low_band_temperature/state";
const String LBAND_TEMP_SET_TOPIC = BASE_TOPIC + "low_band_temperature/set";

const String HBAND_TEMP_STATE_TOPIC = BASE_TOPIC + "high_band_temperature/state";
const String HBAND_TEMP_SET_TOPIC = BASE_TOPIC + "high_band_temperature/set";

const String NOSP_OVERRIDE_TEMP_STATE_TOPIC = BASE_TOPIC + "no_sp_temp_override/state";
const String NOSP_OVERRIDE_TEMP_SET_TOPIC = BASE_TOPIC + "no_sp_temp_override/set";

// boiler water temperature topic
const String TEMP_BOILER_STATE_TOPIC = BASE_TOPIC + "boiler_temperature/state";
const String TEMP_BOILER_TARGET_TEMP_STATE_TOPIC = BASE_TOPIC + "boiler_target_temperature/state";

const String TEMP_BOILER_RETURN_STATE_TOPIC = BASE_TOPIC + "boiler_return_temperature/state";
// debug data
const String INTEGRAL_ERROR_STATE_TOPIC = BASE_TOPIC + "integral-error/state";
const String FLAME_STATUS_STATE_TOPIC = BASE_TOPIC + "flame_status/state";

const String FLAME_LEVEL_STATE_TOPIC = BASE_TOPIC + "flame_level/state";

// domestic hot water temperature topic
const String TEMP_DHW_STATE_TOPIC = BASE_TOPIC + "dhw-temperature/state";
const String TEMP_DHW_SET_TOPIC = BASE_TOPIC + "dhw-temperature/set";

const String ACTUAL_TEMP_DHW_STATE_TOPIC = BASE_TOPIC + "dhw-actual-temperature/state";

// domestic hot water enable/disable
const String STATE_DHW_GET_TOPIC = BASE_TOPIC + "dhw-state/get";
const String STATE_DHW_SET_TOPIC = BASE_TOPIC + "dhw-state/set";

// setpoint topic
const String SETPOINT_OVERRIDE_SET_TOPIC = BASE_TOPIC + "setpoint-override/set";
const String SETPOINT_OVERRIDE_RESET_TOPIC = BASE_TOPIC + "setpoint-override/reset";

const String WIFI_SSID_STATE_TOPIC = BASE_TOPIC + "wifi_ssid/state";
const String WIFI_RSSI_STATE_TOPIC = BASE_TOPIC + "wifi_rssi/state";

const String IP_ADDR_STATE_TOPIC = BASE_TOPIC + "ip_addr/state";
const String MAC_ADDR_STATE_TOPIC = BASE_TOPIC + "mac_addr/state";

const String PING_ALIVE_STATE_TOPIC = BASE_TOPIC + "ping_alive/state";


// logs topic
const String LOG_GET_TOPIC = BASE_TOPIC + "log";
#endif


