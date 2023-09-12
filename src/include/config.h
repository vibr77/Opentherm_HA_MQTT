
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

static const char * SW_VERSION="0.34";

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

static const String BASE_TOPIC = "homeassistant/opentherm-thermostat/";

static const String CLIMATE_BASE_TOPIC = "homeassistant/climate/opentherm-thermostat/";
static const String DISCOVERY_CLIMATE_TOPIC = CLIMATE_BASE_TOPIC + "config";


static const String FLAME_BASE_TOPIC = "homeassistant/binary_sensor/opentherm-thermostat/flame/";
static const String DISCOVERY_FLAME_TOPIC = FLAME_BASE_TOPIC + "config";

static const String WATER_HEATING_BASE_TOPIC = "homeassistant/binary_sensor/opentherm-thermostat/waterheating/";
static const String DISCOVERY_WATER_HEATING_TOPIC = WATER_HEATING_BASE_TOPIC + "config";

static const String CENTRAL_HEATING_BASE_TOPIC = "homeassistant/binary_sensor/opentherm-thermostat/centralheating/";
static const String DISCOVERY_CENTRAL_HEATING_TOPIC = CENTRAL_HEATING_BASE_TOPIC + "config";

static const String BOILERTEMP_BASE_TOPIC = "homeassistant/sensor/opentherm-thermostat/boilertemp/";
static const String DISCOVERY_BOILER_TEMP_TOPIC = BOILERTEMP_BASE_TOPIC + "config";

static const String MODLVL_BASE_TOPIC = "homeassistant/number/opentherm-thermostat/modulationlevel/";
static const String DISCOVERY_MODLVL_TOPIC = MODLVL_BASE_TOPIC + "config";

static const String ENABLE_CHEATING_BASE_TOPIC = "homeassistant/switch/opentherm-thermostat/enable_cheating/";
static const String DISCOVERY_ENABLE_CHEATING_TOPIC = ENABLE_CHEATING_BASE_TOPIC + "config";

static const String ENABLE_WHEATING_BASE_TOPIC = "homeassistant/switch/opentherm-thermostat/enable_wheating/";
static const String DISCOVERY_ENABLE_WHEATING_TOPIC = ENABLE_WHEATING_BASE_TOPIC + "config";

static const String ENABLE_OT_LOG_BASE_TOPIC = "homeassistant/switch/opentherm-thermostat/enable_ot_log/";
static const String DISCOVERY_ENABLE_OT_LOG_TOPIC = ENABLE_OT_LOG_BASE_TOPIC + "config";

static const String OT_LOG_BASE_TOPIC = "homeassistant/sensor/opentherm-thermostat/ot_log/";
static const String DISCOVERY_OT_LOG_TOPIC = OT_LOG_BASE_TOPIC + "config";

static const char * TEMPERATURE_UNIT= "C";
static const float MIN_SP_TEMP=5.0;
static const float MAX_SP_TEMP=30.0;
static const float INITIAL_TEMP=19.0;
static const float PRECISION=0.5;
static const float TEMPERATURE_STEP=0.5;
static  const boolean OPTIMISTIC=false;


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

const String MODULATION_LEVEL_STATE_TOPIC = BASE_TOPIC + "modulation_level/state";
const String MODULATION_LEVEL_SET_TOPIC = BASE_TOPIC + "modulation_level/set";

// boiler water temperature topic
const String TEMP_BOILER_GET_TOPIC = BASE_TOPIC + "boiler-temperature/state";
const String TEMP_BOILER_TARGET_GET_TOPIC = BASE_TOPIC + "boiler-target-temperature/get";

// debug data
const String INTEGRAL_ERROR_GET_TOPIC = BASE_TOPIC + "integral-error/get";
const String FLAME_STATUS_GET_TOPIC = BASE_TOPIC + "flame-status/state";
const String FLAME_LEVEL_GET_TOPIC = BASE_TOPIC + "flame-level/get";

// domestic hot water temperature topic
const String TEMP_DHW_GET_TOPIC = BASE_TOPIC + "dhw-temperature/get";
const String TEMP_DHW_SET_TOPIC = BASE_TOPIC + "dhw-temperature/set";
const String ACTUAL_TEMP_DHW_GET_TOPIC = BASE_TOPIC + "dhw-actual-temperature/get";

// domestic hot water enable/disable
const String STATE_DHW_GET_TOPIC = BASE_TOPIC + "dhw-state/get";
const String STATE_DHW_SET_TOPIC = BASE_TOPIC + "dhw-state/set";

// setpoint topic
const String SETPOINT_OVERRIDE_SET_TOPIC = BASE_TOPIC + "setpoint-override/set";
const String SETPOINT_OVERRIDE_RESET_TOPIC = BASE_TOPIC + "setpoint-override/reset";

// logs topic
const String LOG_GET_TOPIC = BASE_TOPIC + "log";
#endif


// SAMPLE 

/*

{
  "name": "Boiler Thermostat",
  "unit_of_meas": "%",
  "dev_cla": "climate",
  "frc_upd": true,
  "val_tpl": "{{ value_json.value }}",
  "uniq_id": "OT_B_01",
  "availability": {
    "topic": "homeassistant/sensor/opentherm-thermostat/status",
    "payload_available": "ONLINE",
    "payload_not_available": "OFFLINE",
    "value_template": "{{ value_json.status }}"
  },
  "precision": 0.5,
  "initial": 19,
  "min_temp": 5,
  "max_temp": 30,
  "temperature_unit": "C",
  "temp_step": 0.5,
  "optimistic": false,
  "current_temperature_topic": "homeassistant/sensor/opentherm-thermostat/current-temperature/state",
  "current_temperature_template": "{{ value_json.temperature }}",
  "temperature_command_topic": "homeassistant/sensor/opentherm-thermostat/setpoint-temperature/set",
  "temperature_command_template": "{{ value_json.temperature }}",
  "temperature_state_topic": "homeassistant/sensor/opentherm-thermostat/setpoint-temperature/state",
  "temperature_state_template": "{{ value_json.temperature }}",
  "mode_command_topic": "homeassistant/sensor/opentherm-thermostat/mode/set",
  "mode_command_template": "{{ value_json.mode }}",
  "mode_state_topic": "homeassistant/sensor/opentherm-thermostat/mode/state",
  "mode_state_template": "{{ value_json.mode }}",
  "modes": [
    "heat",
    "off"
  ],
  "dev": {
    "ids": [
      "opentherm-boiler"
    ],
    "name": "OpenTherm Boiler Master Thermostat",
    "mdl": "ESP32_MASTER_OT_01",
    "mf": "DIYLESS",
    "sw": "0.34",
    "hw_version": "1.0"
  }
}

*/