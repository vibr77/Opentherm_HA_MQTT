
/*
__   _____ ___ ___        Author: Vincent BESSON
 \ \ / /_ _| _ ) _ \      Release: 0.42
  \ V / | || _ \   /      Date: 20230930
   \_/ |___|___/_|_\      Description: ESP32 Mini Home Assistant Opentherm Master Thermostat
                2023      Licence: Creative Commons
______________________
*/ 
#include <Arduino.h>
#include "pwd.h"
#pragma once

#ifndef _CONFIG_
#define _CONFIG_

static const char * SW_VERSION="0.51";

// MOVE TO pwd.h

//static const char* WIFI_SSID="";
//static const char* WIFI_KEY="";

//static const char* MQTT_SERVER = "";
//static const char* MQTT_USER = "";
//static const char* MQTT_PASS = "";

static const int   MQTT_PORT = 1883;

static const char * MQTT_DEVICENAME="opentherm-Boilerprod";
static const char * MQTT_DEV_UNIQUE_ID="OT_B_02";

static const int HTTP_PORT = 80;

// Master OpenTherm Shield pins configuration
static const int OT_IN_PIN = 19;  //for Arduino, 4 for ESP8266 (D2), 21 for ESP32
static const int OT_OUT_PIN = 18; //for Arduino, 5 for ESP8266 (D1), 22 for ESP32

static const int GREEN_LED_PIN = 26;  
static const int BLUE_LED_PIN = 27; 
static const int RED_LED_PIN= 25;

// Temperature sensor pin
static const int ROOM_TEMP_SENSOR_PIN = 14; //for Arduino, 14 for ESP8266 (D5), 18 for ESP32

// Default Value

static const char * TEMPERATURE_UNIT= "C";
static const float MIN_SP_TEMP=5.0;
static const float MAX_SP_TEMP=30.0;
static const float INITIAL_TEMP=19.0;
static const float INITIAL_SP=19.0;
static const float INITIAL_TARGET_DWH_TEMP=50.0;
static const float INITIAL_NO_SP_TEMP_OVERRIDE=25.0;

static const float PRECISION=0.5;
static const float TEMPERATURE_STEP=0.5;
static  const boolean OPTIMISTIC=false;

static const float LOW_BAND_TEMP=20.0;
static const float HIGH_BAND_TEMP=65;
static const float NO_SP_TEMP_OVERRIDE=25;
static const int MAX_MODULATION_LEVEL=100;

static const char * BASE_TOPIC="homeassistant/opentherm-thermostat/";

static const char * DISCOVERY_CLIMATE_TOPIC =       "homeassistant/climate/opentherm-thermostat/config";
static const char * DISCOVERY_FLAME_TOPIC =         "homeassistant/binary_sensor/opentherm-thermostat/flame/config";
static const char * DISCOVERY_FLAME_LEVEL_TOPIC =   "homeassistant/sensor/opentherm-thermostat/flame_level/config";
static const char * DISCOVERY_WATER_HEATING_TOPIC =  "homeassistant/binary_sensor/opentherm-thermostat/waterheating/config";

static const char * DISCOVERY_CENTRAL_HEATING_TOPIC ="homeassistant/binary_sensor/opentherm-thermostat/centralheating/config";
static const char * DISCOVERY_BOILER_TEMP_TOPIC= "homeassistant/sensor/opentherm-thermostat/boilertemp/config";
static const char * DISCOVERY_BOILER_RETURN_TEMP_TOPIC="homeassistant/sensor/opentherm-thermostat/boiler_return_temp/";
static const char * DISCOVERY_BOILER_TARGET_TEMP_TOPIC= "homeassistant/sensor/opentherm-thermostat/boiler_target_temp/config";

static const char * DISCOVERY_INTERNAL_TEMP_TOPIC= "homeassistant/sensor/opentherm-thermostat/internal_temp/config";
static const char * DISCOVERY_INTERNAL_HUMIDITY_TEMP_TOPIC= "homeassistant/sensor/opentherm-thermostat/internal_humidity/config";

static const char * DISCOVERY_INTEGRAL_ERROR_TOPIC="homeassistant/sensor/opentherm-thermostat/integral_error/config";
static const char * DISCOVERY_DWH_TEMP_TOPIC="homeassistant/sensor/opentherm-thermostat/dwhtemp/config";
static const char * DISCOVERY_MAX_MODLVL_TOPIC="homeassistant/number/opentherm-thermostat/maxmodulationlevel/config";
static const char * DISCOVERY_HBAND_TEMP_TOPIC="homeassistant/number/opentherm-thermostat/high_band_temperature/config";
static const char * DISCOVERY_LBAND_TEMP_TOPIC= "homeassistant/number/opentherm-thermostat/low_band_temperature/config";
static const char * DISCOVERY_NOSP_OVERRIDE_TEMP_TOPIC= "homeassistant/number/opentherm-thermostat/no_sp_temp_override/config";
static const char * DISCOVERY_TEMP_DHW_TOPIC="homeassistant/number/opentherm-thermostat/dwh_temp/config";
static const char * DISCOVERY_ENABLE_CHEATING_TOPIC= "homeassistant/switch/opentherm-thermostat/enable_cheating/config";
static const char * DISCOVERY_ENABLE_WHEATING_TOPIC="homeassistant/switch/opentherm-thermostat/enable_wheating/config";
static const char * DISCOVERY_ENABLE_OT_LOG_TOPIC="homeassistant/switch/opentherm-thermostat/enable_ot_log/config";

static const char * DISCOVERY_WIFI_SSID_TOPIC="homeassistant/sensor/opentherm-thermostat/wifi_ssid/config";
static const char * DISCOVERY_WIFI_RSSI_TOPIC= "homeassistant/sensor/opentherm-thermostat/wifi_rssi/config";
static const char * DISCOVERY_IP_ADDR_TOPIC="homeassistant/sensor/opentherm-thermostat/ip_addr/config";
static const char * DISCOVERY_MAC_ADDR_TOPIC=  "homeassistant/sensor/opentherm-thermostat/mac_addr/config";
static const char * DISCOVERY_PING_ALIVE_TOPIC="homeassistant/sensor/opentherm-thermostat/ping_alive/config";

static const char * DISCOVERY_LEADING_DEVICE_TOPIC="homeassistant/sensor/opentherm-thermostat/leading_device/config";

static const char * DISCOVERY_INIT_DEFAULT_VALUES_TOPIC="homeassistant/button/opentherm-thermostat/init_default_values/config";
static const char * DISCOVERY_IDENTIFY_TOPIC="homeassistant/button/opentherm-thermostat/identify/config";

static const char * DISCOVERY_OT_LOG_TOPIC = "homeassistant/sensor/opentherm-thermostat/ot_log/config";

static const char * AVAILABILITY_TOPIC = "homeassistant/opentherm-thermostat/status";

static const char * INTERNAL_TEMP_STATE_TOPIC = "homeassistant/opentherm-thermostat/internal-temperature/state";
static const char * INTERNAL_HUMIDITY_STATE_TOPIC = "homeassistant/opentherm-thermostat/internal-humidity/state";

static const char *CURRENT_TEMP_STATE_TOPIC = "homeassistant/opentherm-thermostat/current-temperature/state";
static const char * CURRENT_TEMP_SET_TOPIC = "homeassistant/opentherm-thermostat/current-temperature/set";

static const char * LEADING_DEVICE_STATE_TOPIC= "homeassistant/opentherm-thermostat/leading_device/state";
// current temperature topics
static const char * TEMP_SETPOINT_STATE_TOPIC = "homeassistant/opentherm-thermostat/setpoint-temperature/state";
static const char * TEMP_SETPOINT_SET_TOPIC = "homeassistant/opentherm-thermostat/setpoint-temperature/set";

// working mode topics
static const char * MODE_STATE_TOPIC = "homeassistant/opentherm-thermostat/mode/state";
static const char * MODE_SET_TOPIC = "homeassistant/opentherm-thermostat/mode/set";

// Centralheating
static const char * CENTRAL_HEATING_STATE_TOPIC = "homeassistant/opentherm-thermostat/centralheating/state";

// Waterheating
static const char * WATER_HEATING_STATE_TOPIC = "homeassistant/opentherm-thermostat/waterheating/state";
// Enable Central Heating
static const char * ENABLE_CHEATING_STATE_TOPIC = "homeassistant/opentherm-thermostat/enable_cheating/state";
static const char * ENABLE_CHEATING_SET_TOPIC = "homeassistant/opentherm-thermostat/enable_cheating/set";

// Enable Water Heating
static const char * ENABLE_WHEATING_STATE_TOPIC = "homeassistant/opentherm-thermostat/enable_wheating/state";
static const char * ENABLE_WHEATING_SET_TOPIC = "homeassistant/opentherm-thermostat/enable_wheating/set";

// Enable OT Log
static const char * ENABLE_OT_LOG_STATE_TOPIC = "homeassistant/opentherm-thermostat/enable_ot_log/state";
static const char * ENABLE_OT_LOG_SET_TOPIC = "homeassistant/opentherm-thermostat/enable_ot_log/set";

// OT Log
static const char * OT_LOG_STATE_TOPIC = "homeassistant/opentherm-thermostat/ot_log/state";
//static const char * OT_LOG_SET_TOPIC = "ot_log/set";

// Modulation Level

static const char * MAX_MODULATION_LEVEL_STATE_TOPIC = "homeassistant/opentherm-thermostat/max_modulation_level/state";
static const char * MAX_MODULATION_LEVEL_SET_TOPIC = "homeassistant/opentherm-thermostat/max_modulation_level/set";


static const char * LBAND_TEMP_STATE_TOPIC = "homeassistant/opentherm-thermostat/low_band_temperature/state";
static const char * LBAND_TEMP_SET_TOPIC = "homeassistant/opentherm-thermostat/low_band_temperature/set";

static const char * HBAND_TEMP_STATE_TOPIC = "homeassistant/opentherm-thermostat/high_band_temperature/state";
static const char * HBAND_TEMP_SET_TOPIC = "homeassistant/opentherm-thermostat/high_band_temperature/set";

static const char * NOSP_OVERRIDE_TEMP_STATE_TOPIC = "homeassistant/opentherm-thermostat/no_sp_temp_override/state";
static const char * NOSP_OVERRIDE_TEMP_SET_TOPIC = "homeassistant/opentherm-thermostat/no_sp_temp_override/set";

// boiler water temperature topic
static const char * TEMP_BOILER_STATE_TOPIC = "homeassistant/opentherm-thermostat/boiler_temperature/state";
static const char * TEMP_BOILER_TARGET_TEMP_STATE_TOPIC = "homeassistant/opentherm-thermostat/boiler_target_temperature/state";

static const char * TEMP_BOILER_RETURN_STATE_TOPIC = "homeassistant/opentherm-thermostat/boiler_return_temperature/state";
// debug data
static const char * INTEGRAL_ERROR_STATE_TOPIC = "homeassistant/opentherm-thermostat/integral-error/state";
static const char * FLAME_STATUS_STATE_TOPIC = "homeassistant/opentherm-thermostat/flame_status/state";

static const char * FLAME_LEVEL_STATE_TOPIC = "homeassistant/opentherm-thermostat/flame_level/state";

// domestic hot water temperature topic
static const char * TEMP_DHW_STATE_TOPIC = "homeassistant/opentherm-thermostat/dhw-temperature/state";
static const char * TEMP_DHW_SET_TOPIC = "homeassistant/opentherm-thermostat/dhw-temperature/set";

static const char * ACTUAL_TEMP_DHW_STATE_TOPIC = "homeassistant/opentherm-thermostat/dhw-actual-temperature/state";

// domestic hot water enable/disable
static const char * STATE_DHW_GET_TOPIC = "homeassistant/opentherm-thermostat/dhw-state/get";
static const char * STATE_DHW_SET_TOPIC = "homeassistant/opentherm-thermostat/dhw-state/set";

// setpoint topic
static const char * SETPOINT_OVERRIDE_SET_TOPIC = "homeassistant/opentherm-thermostat/setpoint-override/set";
static const char * SETPOINT_OVERRIDE_RESET_TOPIC = "homeassistant/opentherm-thermostat/setpoint-override/reset";

static const char * WIFI_SSID_STATE_TOPIC = "homeassistant/opentherm-thermostat/wifi_ssid/state";
static const char * WIFI_RSSI_STATE_TOPIC = "homeassistant/opentherm-thermostat/wifi_rssi/state";

static const char * IP_ADDR_STATE_TOPIC = "homeassistant/opentherm-thermostat/ip_addr/state";
static const char * MAC_ADDR_STATE_TOPIC = "homeassistant/opentherm-thermostat/mac_addr/state";

static const char * PING_ALIVE_STATE_TOPIC = "homeassistant/opentherm-thermostat/ping_alive/state";
static const char * INIT_DEFAULT_VALUES_TOPIC = "homeassistant/opentherm-thermostat/init_default_value/state";
static const char * IDENTIFY_TOPIC = "homeassistant/opentherm-thermostat/identify/state";

// logs topic
static const char * LOG_GET_TOPIC = "homeassistant/opentherm-thermostat/log";
#endif


