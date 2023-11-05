

#include <Arduino.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <RemoteDebug.h>

#include "include/mqtt_com.h"
#include "esp_log.h"

extern PubSubClient client;
extern RemoteDebug Debug;

extern float oplo,ophi,sp,t,t_ext,ierr,op,Ki,Kd,Kp,Ke;
extern float pid_interval;
extern bool bCentralHeating, bWaterHeatingEnable,bCentralHeatingEnable,bCentralHeatingSw, bHeatingMode,bOtLogEnable,bExtTempEnable;
extern bool bWaterHeating;
extern float dwhTarget;
extern float dwhTemp;
extern float MaxModLevel;
extern float nosp_override;
extern bool bParamChanged;
extern float flameLevel;
extern unsigned long lastSpSet,lastPSet;
extern bool bForceCycle;

#define LOG_TAG "mqtt"

void LogMasterParam(){

  LOGI(LOG_TAG,"Master Param oplo:[%f]",oplo);
  LOGI(LOG_TAG,"Master Param ophi:[%f]",ophi);
  LOGI(LOG_TAG,"Master Param sp:[%f]",sp);
  LOGI(LOG_TAG,"Master Param t:[%f]",t);
  LOGI(LOG_TAG,"Master Param ierr:[%f]",ierr);
  LOGI(LOG_TAG,"Master Param op:[%f]",op);
  LOGI(LOG_TAG,"Master Param nosp:[%f]",nosp_override);
  LOGI(LOG_TAG,"Master Param MaxModLevel:[%f]",MaxModLevel);
  LOGI(LOG_TAG,"Master Param dwhTarget:[%f]",dwhTarget);
  LOGI(LOG_TAG,"Master Param dwhTemp:[%f]",dwhTemp);
  LOGI(LOG_TAG,"Master Param FlameLevel:[%f]",flameLevel);
  LOGI(LOG_TAG,"Master Param pid_interval:[%f]",pid_interval);


  if (bWaterHeatingEnable==true){
    LOGI(LOG_TAG,"Master Param bWaterHeatingEnable:[true]");
  }else{
    LOGI(LOG_TAG,"Master Param bWaterHeatingEnable:[false]");
  }
  if (bCentralHeatingEnable==true){
    LOGI(LOG_TAG,"Master Param bCentralHeatingEnable:[true]");
  }else{
    LOGI(LOG_TAG,"Master Param bCentralHeatingEnable:[false]");
  }

  if (bHeatingMode==true){
    LOGI(LOG_TAG,"Master Param bHeatingMode:[true]");
  }else{
    LOGI(LOG_TAG,"Master Param bHeatingMode:[false]");
  }

  if (bCentralHeatingSw==true){
    LOGI(LOG_TAG,"Master Param bCentralHeatingSw:[true]");
  }else{
    LOGI(LOG_TAG,"Master Param bCentralHeatingSw:[false]");
  }

}


DynamicJsonDocument getDeviceBlock(){
    
    DynamicJsonDocument doc(1024);
    
    doc["dev"]["ids"][0]=MQTT_DEVICENAME;
    doc["dev"]["name"]="Boiler controller ";
    doc["dev"]["mdl"]="ESP32_MASTER_OT_01";
    doc["dev"]["mf"]="VIBR";
    doc["dev"]["sw"]=SW_VERSION;
    doc["dev"]["hw_version"]="1.1";

  
    doc["availability"]["topic"]=AVAILABILITY_TOPIC;
    doc["availability"]["payload_available"]="ONLINE";
    doc["availability"]["payload_not_available"]="OFFLINE";

    return doc;
}

boolean sendMqttMsg(const char* topic,DynamicJsonDocument doc){
    String jsonBuffer;
    size_t n = serializeJson(doc, jsonBuffer);
    bool published=client.publish(topic, jsonBuffer.c_str(), n);
    return published;
}

//
// Boiler Thermostat
//

void MQTT_DiscoveryMsg_Climate(){
  
  DynamicJsonDocument doc(4096);

  doc["name"] = "Boiler";
  doc["dev_cla"] = "climate";
  
  char ID[64];
  sprintf(ID,"%s_CLIMATE",MQTT_DEV_UNIQUE_ID);
  doc["uniq_id"]=ID;

  doc["precision"]=PRECISION;
  doc["initial"]=INITIAL_TEMP;
  doc["min_temp"]=MIN_SP_TEMP;
  doc["max_temp"]=MAX_SP_TEMP;
  doc["temperature_unit"]= TEMPERATURE_UNIT;
  doc["temp_step"]= TEMPERATURE_STEP;
  doc["optimistic"]=OPTIMISTIC;
  doc["retain"]=true;
  doc["qos"]=0;
  doc["current_temperature_topic"]=CURRENT_TEMP_STATE_TOPIC;
  doc["current_temperature_template"]="{{ value_json.temp }}";
  
  doc["temperature_command_topic"]=TEMP_SETPOINT_SET_TOPIC;
  
  doc["temperature_state_topic"]=TEMP_SETPOINT_STATE_TOPIC;
  doc["temperature_state_template"]="{{ value_json.temp }}";

  doc["mode_command_topic"]=MODE_SET_TOPIC;
  doc["mode_state_topic"]=MODE_STATE_TOPIC;
  doc["mode_state_template"]="{{ value_json.mode }}";
  
  doc["modes"][0]="heat";
  doc["modes"][1]="off";
 
  DynamicJsonDocument dev=getDeviceBlock();
  doc["dev"]=dev["dev"];
  doc["availability"]=dev["availability"];

  bool published= sendMqttMsg(DISCOVERY_CLIMATE_TOPIC,doc);
}

//
// Boiler Flaming sensor True | False
//

void MQTT_DiscoveryMsg_Sensor_Flaming(){

  DynamicJsonDocument doc(2048);
 
  doc["name"] = "Flame";
  char ID[64];
  sprintf(ID,"%s_ISFLAME",MQTT_DEV_UNIQUE_ID);
  doc["uniq_id"]=ID;

  doc["icon"]="mdi:fire";
  doc["payload_off"]="OFF";
  doc["payload_on"]="ON";
  doc["qos"]=0;
  doc["state_topic"]=FLAME_STATUS_STATE_TOPIC;
  doc["value_template"]="{{ value_json.value }}";
  
  DynamicJsonDocument dev=getDeviceBlock();
  doc["dev"]=dev["dev"];
  doc["availability"]=dev["availability"];

  bool published= sendMqttMsg(DISCOVERY_FLAME_TOPIC,doc);
}

//
// Boiler Flame Level (Mddulation current level) 0-100%
//

void MQTT_DiscoveryMsg_Sensor_FlameLevel(){

  DynamicJsonDocument doc(4096);

  doc["name"] = "Flame level";
  doc["unit_of_measurement"] = "%";
  doc["icon"]="mdi:fire";
  
  char ID[64];
  sprintf(ID,"%s_FLAMELVL",MQTT_DEV_UNIQUE_ID);
  doc["uniq_id"]=ID;

  doc["qos"]=0;
  doc["state_topic"]=FLAME_LEVEL_STATE_TOPIC;
  doc["value_template"]="{{ value_json.value }}";
  
  DynamicJsonDocument dev=getDeviceBlock();
  doc["dev"]=dev["dev"];
  doc["availability"]=dev["availability"];

  bool published= sendMqttMsg(DISCOVERY_FLAME_LEVEL_TOPIC,doc);

}

//
// Boiler Central Heating Sensor True | False
//

void MQTT_DiscoveryMsg_Sensor_CentralHeating(){

  DynamicJsonDocument doc(2048);

  doc["name"] = "Central Heating";
  char ID[64];
  sprintf(ID,"%s_CENTRALH",MQTT_DEV_UNIQUE_ID);
  doc["uniq_id"]=ID;
  doc["icon"]="mdi:water-boiler";
  doc["payload_off"]="OFF";
  doc["payload_on"]="ON";
  doc["qos"]=0;
  doc["state_topic"]=CENTRAL_HEATING_STATE_TOPIC;
  doc["value_template"]="{{ value_json.value }}";
  
  DynamicJsonDocument dev=getDeviceBlock();
  doc["dev"]=dev["dev"];
  doc["availability"]=dev["availability"];

  bool published= sendMqttMsg(DISCOVERY_CENTRAL_HEATING_TOPIC,doc);
}


//
// Boiler Domestic Water Heating Sensor True | False
//

void MQTT_DiscoveryMsg_Sensor_WaterHeating(){

  DynamicJsonDocument doc(2048);

  doc["name"] = "DHW Heating";
  char ID[64];
  sprintf(ID,"%s_WATERH",MQTT_DEV_UNIQUE_ID);
  doc["uniq_id"]=ID;

  doc["icon"]="mdi:water-boiler";
  doc["payload_off"]="OFF";
  doc["payload_on"]="ON";
  doc["qos"]=0;
  doc["state_topic"]=WATER_HEATING_STATE_TOPIC;
  doc["value_template"]="{{ value_json.value }}";
  
  DynamicJsonDocument dev=getDeviceBlock();
  doc["dev"]=dev["dev"];
  doc["availability"]=dev["availability"];

  bool published= sendMqttMsg(DISCOVERY_WATER_HEATING_TOPIC,doc);
}


//
// Controller humidity from the Internal sensor (HTU31D)
//

void MQTT_DiscoveryMsg_Sensor_InternalHumidity(){

  DynamicJsonDocument doc(2048);

  doc["name"] = "Controller humidity";
  doc["dev_cla"] = "Humidity";
  doc["unit_of_measurement"] = "%";
  
  doc["suggested_display_precision"]=2;
  char ID[64];
  sprintf(ID,"%s_INTERNALHUMIDITY",MQTT_DEV_UNIQUE_ID);
  doc["uniq_id"]=ID;
  doc["qos"]=0;
  doc["state_topic"]=INTERNAL_HUMIDITY_STATE_TOPIC;
  doc["value_template"]="{{ value_json.value }}";
  
  DynamicJsonDocument dev=getDeviceBlock();
  doc["dev"]=dev["dev"];
  doc["availability"]=dev["availability"];

  bool published= sendMqttMsg(DISCOVERY_INTERNAL_HUMIDITY_TEMP_TOPIC,doc);

}

//
// Controller temperature from the Internal sensor (HTU31D)
//


void MQTT_DiscoveryMsg_Sensor_InternalTemperature(){

  DynamicJsonDocument doc(2048);

  doc["name"] = "Controller temperature";
  doc["dev_cla"] = "temperature";
  doc["unit_of_measurement"] = "°C";
  
  doc["suggested_display_precision"]=2;
  char ID[64];
  sprintf(ID,"%s_INTERNALTEMP",MQTT_DEV_UNIQUE_ID);
  doc["uniq_id"]=ID;
  doc["qos"]=0;
  doc["state_topic"]=INTERNAL_TEMP_STATE_TOPIC;
  doc["value_template"]="{{ value_json.temp }}";
  
  DynamicJsonDocument dev=getDeviceBlock();
  doc["dev"]=dev["dev"];
  doc["availability"]=dev["availability"];

  bool published= sendMqttMsg(DISCOVERY_INTERNAL_TEMP_TOPIC,doc);

}


//
// Boiler Current Water Temperature
//

void MQTT_DiscoveryMsg_Sensor_BoilerTemperature(){

  DynamicJsonDocument doc(2048);

  doc["name"] = "Boiler temperature";
  doc["dev_cla"] = "temperature";
  doc["unit_of_measurement"] = "°C";
  
  doc["suggested_display_precision"]=2;
  char ID[64];
  sprintf(ID,"%s_BOILERTEMP",MQTT_DEV_UNIQUE_ID);
  doc["uniq_id"]=ID;
  doc["qos"]=0;
  doc["state_topic"]=TEMP_BOILER_STATE_TOPIC;
  doc["value_template"]="{{ value_json.temp }}";
  
  DynamicJsonDocument dev=getDeviceBlock();
  doc["dev"]=dev["dev"];
  doc["availability"]=dev["availability"];

  bool published= sendMqttMsg(DISCOVERY_BOILER_TEMP_TOPIC,doc);

}

//
// External temperature measure to compute PID E
//

void MQTT_DiscoveryMsg_Sensor_ExternalTemperature(){

  DynamicJsonDocument doc(2048);

  doc["name"] = "External temperature";
  doc["dev_cla"] = "temperature";
  doc["unit_of_measurement"] = "°C";
  
  doc["suggested_display_precision"]=2;
  char ID[64];
  sprintf(ID,"%s_EXTTEMP",MQTT_DEV_UNIQUE_ID);
  doc["uniq_id"]=ID;
  doc["qos"]=0;
  doc["state_topic"]=CURRENT_EXTEMP_STATE_TOPIC;
  doc["value_template"]="{{ value_json.temp }}";
  
  DynamicJsonDocument dev=getDeviceBlock();
  doc["dev"]=dev["dev"];
  doc["availability"]=dev["availability"];

  bool published= sendMqttMsg(DISCOVERY_EXTEMP_TOPIC,doc);

}

//
// Boiler Circuit returned temperature sensor °C
//

void MQTT_DiscoveryMsg_Sensor_BoilerReturnTemperature(){

  DynamicJsonDocument doc(2048);

  doc["name"] = "Boiler Return temperature";
  doc["dev_cla"] = "temperature";
  doc["unit_of_measurement"] = "°C";
  
  doc["suggested_display_precision"]=2;
  char ID[64];
  sprintf(ID,"%s_BOILERRETTEMP",MQTT_DEV_UNIQUE_ID);
  doc["uniq_id"]=ID;

  doc["qos"]=0;
  doc["state_topic"]=TEMP_BOILER_RETURN_STATE_TOPIC;
  doc["value_template"]="{{ value_json.temp }}";
  
  DynamicJsonDocument dev=getDeviceBlock();
  doc["dev"]=dev["dev"];
  doc["availability"]=dev["availability"];

  bool published= sendMqttMsg(DISCOVERY_BOILER_RETURN_TEMP_TOPIC,doc);

}

//
// Boiler Target Water temperature sensor compute by PID+E
//


void MQTT_DiscoveryMsg_Sensor_BoilerTargetTemperature(){

  DynamicJsonDocument doc(4096);

  doc["name"] = "Boiler target temperature";
  doc["dev_cla"] = "temperature";
  doc["unit_of_measurement"] = "°C";
  doc["suggested_display_precision"]=2;
  char ID[64];
  sprintf(ID,"%s_BOILERTARGETTEMP",MQTT_DEV_UNIQUE_ID);
  doc["uniq_id"]=ID;
  
  doc["qos"]=0;
  doc["state_topic"]=TEMP_BOILER_TARGET_TEMP_STATE_TOPIC;
  doc["value_template"]="{{ value_json.temp }}";
  DynamicJsonDocument dev=getDeviceBlock();
  doc["dev"]=dev["dev"];
  doc["availability"]=dev["availability"];
  

  bool published= sendMqttMsg(DISCOVERY_BOILER_TARGET_TEMP_TOPIC,doc);
}

//
// Current Valve leading the heating in the premise (string)
//

void MQTT_DiscoveryMsg_Sensor_LeadingDevice(){
  
  DynamicJsonDocument doc(2048);

  doc["name"] = "Leading device";
  char ID[64];
  sprintf(ID,"%s_LEADING_DEVICE",MQTT_DEV_UNIQUE_ID);
  doc["uniq_id"]=ID;

  doc["icon"]="mdi:star-shooting-outline";
  
  doc["qos"]=0;
  doc["retain"]=true;
 
  doc["state_topic"]=LEADING_DEVICE_STATE_TOPIC;
  doc["value_template"]="{{ value_json.value }}";
  
  DynamicJsonDocument dev=getDeviceBlock();
  doc["dev"]=dev["dev"];
  doc["availability"]=dev["availability"];

  bool published= sendMqttMsg(DISCOVERY_LEADING_DEVICE_TOPIC,doc);

}

//
// Boiler PID I=Intergral Contribution
//


void MQTT_DiscoveryMsg_Sensor_IntegralContribution(){

  DynamicJsonDocument doc(2048);

  doc["name"] = "PID integral contrib.";
  char ID[64];
  sprintf(ID,"%s_IERR",MQTT_DEV_UNIQUE_ID);
  doc["uniq_id"]=ID;

  doc["dev_cla"] = "temperature";
  doc["qos"]=0;
  doc["state_topic"]=INTEGRAL_ERROR_STATE_TOPIC;
  doc["unit_of_measurement"] = "°C";
  doc["value_template"]="{{ value_json.value }}";
  doc["suggested_display_precision"]=2;
  DynamicJsonDocument dev=getDeviceBlock();
  doc["dev"]=dev["dev"];
  doc["availability"]=dev["availability"];
  
  bool published= sendMqttMsg(DISCOVERY_INTEGRAL_ERROR_TOPIC,doc);

}

//
// Boiler PID D=Proportional Contribution
//

void MQTT_DiscoveryMsg_Sensor_ProportionalContribution(){

  DynamicJsonDocument doc(2048);

  doc["name"] = "PID proportional contrib.";
  char ID[64];
  sprintf(ID,"%s_PERR",MQTT_DEV_UNIQUE_ID);
  doc["uniq_id"]=ID;

  doc["dev_cla"] = "temperature";
  doc["qos"]=0;
  doc["state_topic"]=PROPORTIONAL_ERROR_STATE_TOPIC;
  doc["unit_of_measurement"] = "°C";
  doc["value_template"]="{{ value_json.value }}";
  doc["suggested_display_precision"]=2;
  DynamicJsonDocument dev=getDeviceBlock();
  doc["dev"]=dev["dev"];
  doc["availability"]=dev["availability"];
  
  bool published= sendMqttMsg(DISCOVERY_PROPORTIONAL_ERROR_TOPIC,doc);

}

//
// Boiler PID D=Derivative Contribution
//

void MQTT_DiscoveryMsg_Sensor_DerivativeContribution(){

  DynamicJsonDocument doc(2048);

  doc["name"] = "PID derivative contrib.";
  char ID[64];
  sprintf(ID,"%s_DERR",MQTT_DEV_UNIQUE_ID);
  doc["uniq_id"]=ID;

  doc["dev_cla"] = "temperature";
  doc["qos"]=0;
  doc["state_topic"]=DERIVATIVE_ERROR_STATE_TOPIC;
  doc["unit_of_measurement"] = "°C";
  doc["value_template"]="{{ value_json.value }}";
  doc["suggested_display_precision"]=2;
  DynamicJsonDocument dev=getDeviceBlock();
  doc["dev"]=dev["dev"];
  doc["availability"]=dev["availability"];
  
  bool published= sendMqttMsg(DISCOVERY_DERIVATIVE_ERROR_TOPIC,doc);

}

//
// Boiler PID E=External Contribution
//

void MQTT_DiscoveryMsg_Sensor_ExternalContribution(){

  DynamicJsonDocument doc(2048);

  doc["name"] = "PID external contrib.";
  char ID[64];
  sprintf(ID,"%s_EERR",MQTT_DEV_UNIQUE_ID);
  doc["uniq_id"]=ID;

  doc["dev_cla"] = "temperature";
  doc["qos"]=0;
  doc["state_topic"]=EXTERNAL_ERROR_STATE_TOPIC;
  doc["unit_of_measurement"] = "°C";
  doc["value_template"]="{{ value_json.value }}";
  doc["suggested_display_precision"]=2;
  DynamicJsonDocument dev=getDeviceBlock();
  doc["dev"]=dev["dev"];
  doc["availability"]=dev["availability"];
  
  bool published= sendMqttMsg(DISCOVERY_EXTERNAL_ERROR_TOPIC,doc);

}

//
// Boiler Current Domestic Water Heating temperature sensor 
//

void MQTT_DiscoveryMsg_Sensor_dwhTemperature(){

  DynamicJsonDocument doc(2048);

  doc["name"] = "DHW temperature";
  doc["dev_cla"] = "temperature";
  doc["unit_of_measurement"] = "°C";
  doc["suggested_display_precision"]=2;
  char ID[64];
  sprintf(ID,"%s_DWHTEMP",MQTT_DEV_UNIQUE_ID);
  doc["uniq_id"]=ID;

  doc["qos"]=0;
  doc["state_topic"]=ACTUAL_TEMP_DHW_STATE_TOPIC;
  doc["value_template"]="{{ value_json.temp }}";
  
  DynamicJsonDocument dev=getDeviceBlock();
  doc["dev"]=dev["dev"];
  doc["availability"]=dev["availability"];
  
  bool published= sendMqttMsg(DISCOVERY_DWH_TEMP_TOPIC,doc);

}

//
// Boiler [Config] Domestic Water Heating Max/Target temperature
//


void MQTT_DiscoveryMsg_Number_dwh_temp(){

  DynamicJsonDocument doc(2048);

  doc["name"] = "Domestic Hot Water";
  char ID[64];
  sprintf(ID,"%s_DWH_TEMP",MQTT_DEV_UNIQUE_ID);
  doc["uniq_id"]=ID;

  doc["icon"]="mdi:cup-water";
  doc["unit_of_measurement"]="°C";
  doc["min"]=20;
  doc["max"]=70;
  doc["mode"]="slider";
  doc["step"]=1;
  doc["qos"]=2;
  doc["retain"]=true;
  doc["entity_category"]="config";
  doc["optimistic"]=OPTIMISTIC;
  doc["state_topic"]=TEMP_DHW_STATE_TOPIC;
  doc["value_template"]="{{ value_json.temp }}";
  
  doc["command_topic"]=TEMP_DHW_SET_TOPIC;
  
  DynamicJsonDocument dev=getDeviceBlock();
  doc["dev"]=dev["dev"];
  doc["availability"]=dev["availability"];

  bool published= sendMqttMsg(DISCOVERY_TEMP_DHW_TOPIC,doc);
}

//
// Boiler [Config] PID Interval compute (sec)
//

void MQTT_DiscoveryMsg_Number_PID_Interval(){

  DynamicJsonDocument doc(2048);

  doc["name"] = "PID interval";
  char ID[64];
  sprintf(ID,"%s_PIDInterval",MQTT_DEV_UNIQUE_ID);
  doc["uniq_id"]=ID;
  doc["icon"]="mdi:sine-wave";

  doc["min"]=1;
  doc["max"]=600;
  doc["mode"]="box";
  doc["step"]=1;
  doc["qos"]=0;
  doc["retain"]=true;
  doc["unit_of_measurement"]="s";
  doc["entity_category"]="config";
  
  doc["state_topic"]=PID_INTERVAL_STATE_TOPIC;
  doc["value_template"]="{{ value_json.value }}";
  doc["command_topic"]=PID_INTERVAL_SET_TOPIC;

  DynamicJsonDocument dev=getDeviceBlock();
  doc["dev"]=dev["dev"];
  doc["availability"]=dev["availability"];

  bool published= sendMqttMsg(DISCOVERY_PID_INTERVAL,doc);
}

//
// Boiler [Config] PID Constant Kp
//

void MQTT_DiscoveryMsg_Number_PID_Kp(){

  DynamicJsonDocument doc(2048);

  doc["name"] = "PID Kp";
  char ID[64];
  sprintf(ID,"%s_PIDKP",MQTT_DEV_UNIQUE_ID);
  doc["uniq_id"]=ID;
  doc["icon"]="mdi:sine-wave";

  doc["min"]=1;
  doc["max"]=100;
  doc["mode"]="box";
  doc["step"]=1;
  doc["qos"]=0;
  doc["retain"]=true;
 
  doc["entity_category"]="config";
  
  doc["state_topic"]=PID_KP_STATE_TOPIC;
  doc["value_template"]="{{ value_json.value }}";
  doc["command_topic"]=PID_KP_SET_TOPIC;
  DynamicJsonDocument dev=getDeviceBlock();
  doc["dev"]=dev["dev"];
  doc["availability"]=dev["availability"];

  bool published= sendMqttMsg(DISCOVERY_PID_KP_TOPIC,doc);
}

//
// Boiler [Config] PID Constant Ki
//

void MQTT_DiscoveryMsg_Number_PID_Ki(){

  DynamicJsonDocument doc(2048);

  doc["name"] = "PID Ki";
  char ID[64];
  sprintf(ID,"%s_PIDKI",MQTT_DEV_UNIQUE_ID);
  doc["uniq_id"]=ID;
  doc["icon"]="mdi:sine-wave";

  doc["min"]=0;
  doc["max"]=1;
  doc["mode"]="box";
  doc["step"]=0.001;
  doc["qos"]=0;
  doc["retain"]=true;
 
  doc["entity_category"]="config";
  
  doc["state_topic"]=PID_KI_STATE_TOPIC;
  doc["value_template"]="{{ value_json.value }}";
  doc["command_topic"]=PID_KI_SET_TOPIC;
  
  DynamicJsonDocument dev=getDeviceBlock();
  doc["dev"]=dev["dev"];
  doc["availability"]=dev["availability"];

  bool published= sendMqttMsg(DISCOVERY_PID_KI_TOPIC,doc);
}

//
// Boiler [Config] PID Constant Kd
//

void MQTT_DiscoveryMsg_Number_PID_Kd(){

  DynamicJsonDocument doc(2048);

  doc["name"] = "PID Kd";
  char ID[64];
  sprintf(ID,"%s_PIDKD",MQTT_DEV_UNIQUE_ID);
  doc["uniq_id"]=ID;
  doc["icon"]="mdi:sine-wave";

  doc["min"]=0;
  doc["max"]=10000;
  doc["mode"]="box";
  doc["step"]=1;
  doc["qos"]=0;
  doc["retain"]=true;

  doc["entity_category"]="config";
  
  doc["state_topic"]=PID_KD_STATE_TOPIC;
  doc["value_template"]="{{ value_json.value }}";
  doc["command_topic"]=PID_KD_SET_TOPIC;
  DynamicJsonDocument dev=getDeviceBlock();
  doc["dev"]=dev["dev"];
  doc["availability"]=dev["availability"];

  bool published= sendMqttMsg(DISCOVERY_PID_KD_TOPIC,doc);
}

//
// Boiler [Config] PID Constant Ke
//

void MQTT_DiscoveryMsg_Number_PID_Ke(){

  DynamicJsonDocument doc(2048);

  doc["name"] = "PID Ke";
  char ID[64];
  sprintf(ID,"%s_PIDKE",MQTT_DEV_UNIQUE_ID);
  doc["uniq_id"]=ID;
  doc["icon"]="mdi:sine-wave";

  doc["min"]=0;
  doc["max"]=1;
  doc["mode"]="box";
  doc["step"]=0.001;
  doc["qos"]=0;
  doc["retain"]=true;
  doc["entity_category"]="config";
  
  doc["state_topic"]=PID_KE_STATE_TOPIC;
  doc["value_template"]="{{ value_json.value }}";
  doc["command_topic"]=PID_KE_SET_TOPIC;
  DynamicJsonDocument dev=getDeviceBlock();
  doc["dev"]=dev["dev"];
  doc["availability"]=dev["availability"];

  bool published= sendMqttMsg(DISCOVERY_PID_KE_TOPIC,doc);
}

//
// Boiler [Config] Max Modulation Level % 1-100
//

void MQTT_DiscoveryMsg_Number_MaxModulationLevel(){

  DynamicJsonDocument doc(2048);

  doc["name"] = "Max modulation";
  char ID[64];
  sprintf(ID,"%s_MAXMODLVL",MQTT_DEV_UNIQUE_ID);
  doc["uniq_id"]=ID;
  doc["icon"]="mdi:sine-wave";

  doc["min"]=1;
  doc["max"]=100;
  doc["mode"]="slider";
  doc["step"]=1;
  doc["qos"]=2;
  doc["retain"]=true;
  doc["unit_of_measurement"]="%";
  doc["entity_category"]="config";
  doc["optimistic"]=OPTIMISTIC;
  doc["state_topic"]=MAX_MODULATION_LEVEL_STATE_TOPIC;
  doc["value_template"]="{{ value_json.level }}";
  
  doc["command_topic"]=MAX_MODULATION_LEVEL_SET_TOPIC;
  
  DynamicJsonDocument dev=getDeviceBlock();
  doc["dev"]=dev["dev"];
  doc["availability"]=dev["availability"];

  bool published= sendMqttMsg(DISCOVERY_MAX_MODLVL_TOPIC,doc);
}

//
// Boiler [Config] Heating Wateer Temperature low band value (check your boiler manual)
//

void MQTT_DiscoveryMsg_Number_LowBandTemperature(){

  DynamicJsonDocument doc(2048);

  doc["name"] = "Low band temperature";
  char ID[64];
  sprintf(ID,"%s_LBTEMP",MQTT_DEV_UNIQUE_ID);
  doc["uniq_id"]=ID;

  doc["icon"]="mdi:sine-wave";

  doc["min"]=1;
  doc["max"]=90;
  doc["mode"]="slider";
  doc["step"]=1;
  doc["unit_of_measurement"]="°C";
  doc["qos"]=2;
  doc["retain"]=true;
  doc["entity_category"]="config";
  doc["optimistic"]=OPTIMISTIC;
  doc["state_topic"]=LBAND_TEMP_STATE_TOPIC;
  doc["value_template"]="{{ value_json.temp }}";
  
  doc["command_topic"]=LBAND_TEMP_SET_TOPIC;
  
  DynamicJsonDocument dev=getDeviceBlock();
  doc["dev"]=dev["dev"];
  doc["availability"]=dev["availability"];

  bool published= sendMqttMsg(DISCOVERY_LBAND_TEMP_TOPIC,doc);
}

//
// Boiler [Config] Heating Wateer Temperature high band value (check your boiler manual)
//

void MQTT_DiscoveryMsg_Number_HighBandTemperature(){

  DynamicJsonDocument doc(2048);

  doc["name"] = "High band temperature";
  char ID[64];
  sprintf(ID,"%s_HBTEMP",MQTT_DEV_UNIQUE_ID);
  doc["uniq_id"]=ID;
 
  doc["icon"]="mdi:sine-wave";

  doc["min"]=1;
  doc["max"]=90;
  doc["mode"]="slider";
  doc["step"]=1;
  doc["qos"]=2;
  doc["unit_of_measurement"]="°C";
  doc["retain"]=true;
  doc["entity_category"]="config";
  doc["optimistic"]=OPTIMISTIC;
  doc["state_topic"]=HBAND_TEMP_STATE_TOPIC;
  doc["value_template"]="{{ value_json.temp }}";
  
  doc["command_topic"]=HBAND_TEMP_SET_TOPIC;
  
  DynamicJsonDocument dev=getDeviceBlock();
  doc["dev"]=dev["dev"];
  doc["availability"]=dev["availability"];

  bool published= sendMqttMsg(DISCOVERY_HBAND_TEMP_TOPIC,doc);
}

//
// Boiler [Config] not implemented yet
//

void MQTT_DiscoveryMsg_Number_NospTempOverride(){

  DynamicJsonDocument doc(2048);

  doc["name"] = "No sp temp override";
  char ID[64];
  sprintf(ID,"%s_NOSP_TEMP_OVERRIDE",MQTT_DEV_UNIQUE_ID);
  doc["uniq_id"]=ID;

  doc["icon"]="mdi:sine-wave";

  doc["min"]=1;
  doc["max"]=90;
  doc["mode"]="slider";
  doc["step"]=1;
  doc["qos"]=2;
  doc["unit_of_measurement"]="°C";
  doc["retain"]=true;
  doc["entity_category"]="config";
  doc["optimistic"]=OPTIMISTIC;
  doc["state_topic"]=NOSP_OVERRIDE_TEMP_STATE_TOPIC;
  doc["value_template"]="{{ value_json.temp }}";
  
  doc["command_topic"]=NOSP_OVERRIDE_TEMP_SET_TOPIC;
  
  DynamicJsonDocument dev=getDeviceBlock();
  doc["dev"]=dev["dev"];
  doc["availability"]=dev["availability"];

  bool published= sendMqttMsg(DISCOVERY_NOSP_OVERRIDE_TEMP_TOPIC,doc);
}

//
// Boiler [Config] Enable External temperature compensation PID+E TRUE | FALSE
//

void MQTT_DiscoveryMsg_Switch_EnableExtTemp(){

  DynamicJsonDocument doc(2048);

  doc["name"] = "External temperature";
  doc["icon"]="mdi:fire";
  char ID[64];
  sprintf(ID,"%s_ENABLE_EXTEMP",MQTT_DEV_UNIQUE_ID);
  doc["uniq_id"]=ID;
  doc["device_class"]="switch";
  
  doc["payload_off"]="0";
  doc["payload_on"]="1";

  doc["state_off"]="0";
  doc["state_on"]="1";
  
  doc["qos"]=0;
  doc["retain"]=true;
  doc["entity_category"]="config";
  doc["optimistic"]=OPTIMISTIC;

  doc["state_topic"]=ENABLE_EXTTEMP_STATE_TOPIC;
  doc["command_topic"]=ENABLE_EXTTEMP_SET_TOPIC;
  
  DynamicJsonDocument dev=getDeviceBlock();
  doc["dev"]=dev["dev"];
  doc["availability"]=dev["availability"];

  bool published= sendMqttMsg(DISCOVERY_ENABLE_EXTTEMP_TOPIC,doc);
}

//
// Boiler [Config] Enable Central Heating TRUE|FALSE
//
void MQTT_DiscoveryMsg_Switch_SwCentralHeating(){

  DynamicJsonDocument doc(2048);

  doc["name"] = "SW Central Heating";
  doc["icon"]="mdi:water-boiler";
  char ID[64];
  sprintf(ID,"%s_SW_CHEATING",MQTT_DEV_UNIQUE_ID);
  doc["uniq_id"]=ID;
  doc["device_class"]="switch";
  
  doc["payload_off"]="0";
  doc["payload_on"]="1";

  doc["state_off"]="0";
  doc["state_on"]="1";
  doc["enabled_by_default"]=false;
  doc["qos"]=0;
  doc["retain"]=true;
  doc["entity_category"]="config";
  doc["optimistic"]=OPTIMISTIC;
  
  doc["state_topic"]=SW_CHEATING_STATE_TOPIC;
  doc["command_topic"]=SW_CHEATING_SET_TOPIC;
  
  DynamicJsonDocument dev=getDeviceBlock();
  doc["dev"]=dev["dev"];
  doc["availability"]=dev["availability"];

  bool published= sendMqttMsg(DISCOVERY_SW_CHEATING_TOPIC,doc);
}


void MQTT_DiscoveryMsg_Switch_EnableCentralHeating(){

  DynamicJsonDocument doc(2048);

  doc["name"] = "Central Heating";
  doc["icon"]="mdi:water-boiler";
  char ID[64];
  sprintf(ID,"%s_ENABLE_CHEATING",MQTT_DEV_UNIQUE_ID);
  doc["uniq_id"]=ID;
  doc["device_class"]="switch";
  
  doc["payload_off"]="0";
  doc["payload_on"]="1";

  doc["state_off"]="0";
  doc["state_on"]="1";
  
  doc["qos"]=0;
  doc["retain"]=true;
  doc["entity_category"]="config";
  doc["optimistic"]=OPTIMISTIC;

  doc["state_topic"]=ENABLE_CHEATING_STATE_TOPIC;
  doc["command_topic"]=ENABLE_CHEATING_SET_TOPIC;
  
  DynamicJsonDocument dev=getDeviceBlock();
  doc["dev"]=dev["dev"];
  doc["availability"]=dev["availability"];

  bool published= sendMqttMsg(DISCOVERY_ENABLE_CHEATING_TOPIC,doc);
}

//
// Boiler [Config] Enable DwH Heating TRUE|FALSE
//

void MQTT_DiscoveryMsg_Switch_EnableWaterHeating(){

  DynamicJsonDocument doc(2048);

  doc["name"] = "Domestic Water Heating";
  char ID[64];
  sprintf(ID,"%s_ENABLE_WHEATING",MQTT_DEV_UNIQUE_ID);
  doc["uniq_id"]=ID;

  doc["device_class"]="switch";
  doc["icon"]="mdi:fire";
  
  doc["payload_off"]="0";
  doc["payload_on"]="1";
 

  doc["state_off"]="0";
  doc["state_on"]="1";
  
  doc["qos"]=0;
  doc["retain"]=true;
  doc["entity_category"]="config";
  doc["optimistic"]=OPTIMISTIC;
  
  doc["state_topic"]=ENABLE_WHEATING_STATE_TOPIC;
  doc["command_topic"]=ENABLE_WHEATING_SET_TOPIC;
  
  DynamicJsonDocument dev=getDeviceBlock();
  doc["dev"]=dev["dev"];
  doc["availability"]=dev["availability"];

  bool published= sendMqttMsg(DISCOVERY_ENABLE_WHEATING_TOPIC,doc);
}

//
// Boiler Log Message string sensor
//

void MQTT_DiscoveryMsg_Text_Log(){
  
  DynamicJsonDocument doc(2048);

  doc["name"] = "OpenTherm Log";
  char ID[64];
  sprintf(ID,"%s_OT_LOG",MQTT_DEV_UNIQUE_ID);
  doc["uniq_id"]=ID;

  doc["icon"]="mdi:radiology-box-outline";
  
  doc["qos"]=0;
  doc["retain"]=true;
  doc["entity_category"]="diagnostic";
  doc["state_topic"]=OT_LOG_STATE_TOPIC;
  doc["value_template"]="{{ value_json.text }}";
  
  DynamicJsonDocument dev=getDeviceBlock();
  doc["dev"]=dev["dev"];
  doc["availability"]=dev["availability"];

  bool published= sendMqttMsg(DISCOVERY_OT_LOG_TOPIC,doc);

}

//
// Boiler [Config] Enable message logging TRUE|FALSE
//

void MQTT_DiscoveryMsg_Switch_EnableLog(){

 DynamicJsonDocument doc(2048);

  doc["name"] = "Enable OT Log";
  char ID[64];
  sprintf(ID,"%s_ENABLE_OT_LOG",MQTT_DEV_UNIQUE_ID);
  doc["uniq_id"]=ID;

  doc["device_class"]="switch";
  doc["icon"]="mdi:radiology-box-outline";
  
  doc["payload_off"]="0";
  doc["payload_on"]="1";
 
  doc["state_off"]="0";
  doc["state_on"]="1";
  
  doc["qos"]=0;
  doc["retain"]=true;
  doc["entity_category"]="diagnostic";
  doc["optimistic"]=OPTIMISTIC;
  
  doc["state_topic"]=ENABLE_OT_LOG_STATE_TOPIC;
  doc["command_topic"]=ENABLE_OT_LOG_SET_TOPIC;
  
  DynamicJsonDocument dev=getDeviceBlock();
  doc["dev"]=dev["dev"];
  doc["availability"]=dev["availability"];

  bool published= sendMqttMsg(DISCOVERY_ENABLE_OT_LOG_TOPIC,doc);

}

void MQTT_DiscoveryMsg_Text_WIFI_SSID(){
  
  DynamicJsonDocument doc(2048);

  doc["name"] = "WiFi SSID";
  char ID[64];
  sprintf(ID,"%s_WIFI_SSID",MQTT_DEV_UNIQUE_ID);
  doc["uniq_id"]=ID;

  doc["icon"]="mdi:wifi";
  
  doc["qos"]=0;
  doc["retain"]=true;
  doc["entity_category"]="diagnostic";
  doc["state_topic"]=WIFI_SSID_STATE_TOPIC;
  doc["value_template"]="{{ value_json.value }}";
  
  DynamicJsonDocument dev=getDeviceBlock();
  doc["dev"]=dev["dev"];
  doc["availability"]=dev["availability"];

  bool published= sendMqttMsg(DISCOVERY_WIFI_SSID_TOPIC,doc);

}

void MQTT_DiscoveryMsg_Text_WIFI_RSSI(){
  
  DynamicJsonDocument doc(2048);

  doc["name"] = "WiFi RSSI";
  char ID[64];
  sprintf(ID,"%s_WIFI_RSSI",MQTT_DEV_UNIQUE_ID);
  doc["uniq_id"]=ID;

  doc["icon"]="mdi:wifi-strength-1";
  
  doc["qos"]=0;
  doc["retain"]=true;
  doc["entity_category"]="diagnostic";
  doc["unit_of_measurement"]="dBm";
  doc["state_topic"]=WIFI_RSSI_STATE_TOPIC;
  doc["value_template"]="{{ value_json.value }}";
  
  DynamicJsonDocument dev=getDeviceBlock();
  doc["dev"]=dev["dev"];
  doc["availability"]=dev["availability"];

  bool published= sendMqttMsg(DISCOVERY_WIFI_RSSI_TOPIC,doc);

}

void MQTT_DiscoveryMsg_Text_IpAddr(){
  
  DynamicJsonDocument doc(2048);

  doc["name"] = "IP Addr";
  char ID[64];
  sprintf(ID,"%s_IP_ADDR",MQTT_DEV_UNIQUE_ID);
  doc["uniq_id"]=ID;

  doc["icon"]="mdi:ip-network";
  
  doc["qos"]=0;
  doc["retain"]=true;
  doc["entity_category"]="diagnostic";
  doc["state_topic"]=IP_ADDR_STATE_TOPIC;
  doc["value_template"]="{{ value_json.value }}";
  
  DynamicJsonDocument dev=getDeviceBlock();
  doc["dev"]=dev["dev"];
  doc["availability"]=dev["availability"];

  bool published= sendMqttMsg(DISCOVERY_IP_ADDR_TOPIC,doc);

}

void MQTT_DiscoveryMsg_Text_MacAddr(){
  
  DynamicJsonDocument doc(2048);

  doc["name"] = "Mac Addr";
  char ID[64];
  sprintf(ID,"%s_MAC_ADDR",MQTT_DEV_UNIQUE_ID);
  doc["uniq_id"]=ID;

  doc["icon"]="mdi:web";
  
  doc["qos"]=0;
  doc["retain"]=true;
  doc["entity_category"]="diagnostic";
  doc["state_topic"]=MAC_ADDR_STATE_TOPIC;
  doc["value_template"]="{{ value_json.value }}";
  
  DynamicJsonDocument dev=getDeviceBlock();
  doc["dev"]=dev["dev"];
  doc["availability"]=dev["availability"];

  bool published= sendMqttMsg(DISCOVERY_MAC_ADDR_TOPIC,doc);

}

void MQTT_DiscoveryMsg_Text_PingAlive(){
  
  DynamicJsonDocument doc(2048);

  doc["name"] = "Ping Alive";
  char ID[64];
  sprintf(ID,"%s_PING_ALIVE",MQTT_DEV_UNIQUE_ID);
  doc["uniq_id"]=ID;

  doc["icon"]="mdi:heart-pulse";
  doc["unit_of_measurement"]="min";

  doc["qos"]=0;
  doc["retain"]=true;
  doc["entity_category"]="diagnostic";
  doc["state_topic"]=PING_ALIVE_STATE_TOPIC;
  doc["value_template"]="{{ value_json.value }}";
  
  DynamicJsonDocument dev=getDeviceBlock();
  doc["dev"]=dev["dev"];
  doc["availability"]=dev["availability"];

  bool published= sendMqttMsg(DISCOVERY_PING_ALIVE_TOPIC,doc);

}

void MQTT_DiscoveryMsg_Button_InitDefValues(){
  
  DynamicJsonDocument doc(2048);

  doc["name"] = "Init default values";
  char ID[64];
  sprintf(ID,"%s_INIT_VALUES",MQTT_DEV_UNIQUE_ID);
  doc["uniq_id"]=ID;

  doc["icon"]="mdi:refresh-auto";

  doc["qos"]=0;
  doc["retain"]=false;
  doc["entity_category"]="diagnostic";
  doc["command_topic"]=INIT_DEFAULT_VALUES_TOPIC;
  
  DynamicJsonDocument dev=getDeviceBlock();
  doc["dev"]=dev["dev"];
  doc["availability"]=dev["availability"];

  bool published= sendMqttMsg(DISCOVERY_INIT_DEFAULT_VALUES_TOPIC,doc);

}

void MQTT_DiscoveryMsg_Button_triggerCycle(){
  
  DynamicJsonDocument doc(2048);

  doc["name"] = "Trigger cycle";
  char ID[64];
  sprintf(ID,"%s_TRIGGER_CYCLE",MQTT_DEV_UNIQUE_ID);
  doc["uniq_id"]=ID;

  doc["icon"]="mdi:refresh-auto";

  doc["qos"]=0;
  doc["retain"]=false;
  doc["entity_category"]="diagnostic";
  doc["command_topic"]=TRIGGER_CYCLE_TOPIC;
  
  DynamicJsonDocument dev=getDeviceBlock();
  doc["dev"]=dev["dev"];
  doc["availability"]=dev["availability"];

  bool published= sendMqttMsg(DISCOVERY_TRIGGER_CYCLE_TOPIC,doc);

}

void callback(char* topic, byte* payload, unsigned int length) {
  LOGD(LOG_TAG,"MQTT Callback topic:[%s]",topic);

  char* p = (char*)malloc((length+1)*sizeof(char));
  memcpy(p,payload,length);
  
  p[length]=0;
  bool pubResult=false;

  LOGD(LOG_TAG,"MQTT Callback topic:[%s] payload:[%s]",topic,p);

  if (!strcmp(topic, CURRENT_EXTEMP_STATE_TOPIC)) {
    float t_ext=getPayloadFloatValue("temp",p);
    LOGD(LOG_TAG,"CURRENT_EXTEMP_STATE_TOPIC new value:%f",t_ext);
  }
  // PID Management 
  else if (!strcmp(topic, PID_KP_STATE_TOPIC)) {
    float Kp1=getPayloadFloatValue("temp",p);
    if (Kp1>0) {
      Kp=Kp1;
      LOGD(LOG_TAG,"PID_KP_STATE_TOPIC new value:%f",Kp1);
    }else{
      LOGE(LOG_TAG,"PID_KP_STATE_TOPIC value <= 0 error");
    }
  }else if (!strcmp(topic, PID_KP_SET_TOPIC)) {
    float val = atof(p);
    if (!isnan(val) && isValidNumber(p)) {
      pubResult=publishToTopicFloat(val,PID_KP_STATE_TOPIC,"value",true); // Publish the new température;
      if (pubResult==true){
        LOGD(LOG_TAG,"PID_KP_SET_TOPIC value:[%f]",val);
      }else{
        LOGE(LOG_TAG,"PID_KP_SET_TOPIC publish error");
      }
    }
  }else if (!strcmp(topic, PID_KI_STATE_TOPIC)) {
    float Ki1=getPayloadFloatValue("value",p);
    if (Ki1>0) {
      Ki=Ki1;
      LOGD(LOG_TAG,"PID_KI_STATE_TOPIC new value:%f",Ki1);
    }else{
      LOGE(LOG_TAG,"PID_KI_STATE_TOPIC value <= 0 error");
    }
  }else if (!strcmp(topic, PID_KI_SET_TOPIC)) {
    float val = atof(p);
    if (!isnan(val) && isValidNumber(p)) {
      pubResult=publishToTopicFloat(val,PID_KI_STATE_TOPIC,"value",true); // Publish the new température;
      if (pubResult==true){
        LOGD(LOG_TAG,"PID_KI_SET_TOPIC value:[%f]",val);
      }else{
        LOGE(LOG_TAG,"PID_KI_SET_TOPIC publish error");
      }
    }
  }
  else if (!strcmp(topic, PID_KD_STATE_TOPIC)) {
    float Kd1=getPayloadFloatValue("value",p);
    if (Kd1>0) {
      Kd=Kd1;
      LOGD(LOG_TAG,"PID_KD_STATE_TOPIC new value:%f",Kd1);
    }else{
      LOGE(LOG_TAG,"PID_KD_STATE_TOPIC value <= 0 error");
    }
  }else if (!strcmp(topic, PID_KD_SET_TOPIC)) {
    float val = atof(p);
    if (!isnan(val) && isValidNumber(p)) {
      pubResult=publishToTopicFloat(val,PID_KD_STATE_TOPIC,"value",true); // Publish the new température;
      if (pubResult==true){
        LOGD(LOG_TAG,"PID_KD_SET_TOPIC value:[%f]",val);
      }else{
        LOGE(LOG_TAG,"PID_KD_SET_TOPIC publish error");
      }
    }
  }
  
  else if (!strcmp(topic, PID_KE_STATE_TOPIC)) {
    float Ke1=getPayloadFloatValue("value",p);
    if (Ke1>0) {
      Ke=Ke1;
      LOGD(LOG_TAG,"PID_KD_STATE_TOPIC new value:%f",Ke1);
    }else{
      LOGE(LOG_TAG,"PID_KD_STATE_TOPIC value <= 0 error");
    }
  }
  else if (!strcmp(topic, PID_KE_SET_TOPIC)) {
    float val = atof(p);
    if (!isnan(val) && isValidNumber(p)) {
      pubResult=publishToTopicFloat(val,PID_KE_STATE_TOPIC,"value",true); // Publish the new température;
      if (pubResult==true){
        LOGD(LOG_TAG,"PID_KE_SET_TOPIC value:[%f]",val);
      }else{
        LOGE(LOG_TAG,"PID_KE_SET_TOPIC publish error");
      }
    }
  }
  else if (!strcmp(topic, PID_INTERVAL_STATE_TOPIC)) {
    float val=getPayloadFloatValue("value",p);
    if (val>0 && val<601) {
      pid_interval=val;
      LOGD(LOG_TAG,"PID_INTERVAL_STATE_TOPIC new value:%f",val);
    }else{
      LOGE(LOG_TAG,"PID_INTERVAL_STATE_TOPIC value <= 0 error");
    }
  }
  else if (!strcmp(topic, PID_INTERVAL_SET_TOPIC)) {
    float val=atof(p);
    if (!isnan(val) && isValidNumber(p)) {
      pubResult=publishToTopicFloat(val,PID_INTERVAL_STATE_TOPIC,"value",true); // Publish the new température;
      if (pubResult==true){
        LOGD(LOG_TAG,"PID_INTERVAL_STATE_TOPIC value:[%f]",val);
      }else{
        LOGE(LOG_TAG,"PID_INTERVAL_STATE_TOPIC publish error");
      }
    }
  }

  // CURRENT TEMPERATURE
  else if (!strcmp(topic, CURRENT_TEMP_STATE_TOPIC)) {
    float val=getPayloadFloatValue("temp",p);
    LOGD("MAIN","debug payload: %f",p);
    if (val>0) {
      t=val;
      LOGD(LOG_TAG,"CURRENT_TEMP_STATE_TOPIC new temperature:%f",val);
      unsigned long now = millis();
      lastPSet=now;
    }else{
      LOGE(LOG_TAG,"CURRENT_TEMP_STATE_TOPIC value <= 0 error");
    }
  }
  else if (!strcmp(topic, CURRENT_TEMP_SET_TOPIC)) {
    float t1=atof(p);
    if (!isnan(t1) && isValidNumber(p)) {
      pubResult=publishToTopicFloat(t1,CURRENT_TEMP_STATE_TOPIC,"temp",true); // Publish the new température;
      if (pubResult==true){
        t=t1;
        LOGD(LOG_TAG,"CURRENT_TEMP_SET_TOPIC new temperature:%f",t1);
       
        
      }else{
        LOGE(LOG_TAG,"CURRENT_TEMP_SET_TOPIC publish error");
      }
    }
  }

  // SETPOINT
  else if (!strcmp(topic, TEMP_SETPOINT_STATE_TOPIC)) {
    float sp1=getPayloadFloatValue("temp",p);
    LOGD(LOG_TAG,"TEMP_SETPOINT_STATE_TOPIC payload:[%s] conversion to float:[%f]",p,sp1);
    if (sp1>0) {
      LOGD(LOG_TAG,"TEMP_SETPOINT_STATE_TOPIC new temperature:%f",sp1);
      sp=sp1;
      unsigned long now = millis();
      lastSpSet=now;
    }else{
      LOGE(LOG_TAG,"TEMP_SETPOINT_STATE_TOPIC value <= 0 error");
    }
  
  }else if (!strcmp(topic, TEMP_SETPOINT_SET_TOPIC)) {
    float sp1 = atof(p);
    if (!isnan(sp1) && isValidNumber(p)) {
      
      pubResult=publishToTopicFloat(sp1,TEMP_SETPOINT_STATE_TOPIC,"temp",true); // Publish the new température;
      if (pubResult==true){
        LOGD(LOG_TAG,"TEMP_SETPOINT_SET_TOPIC temperature:[%f]",sp1);
        sp=sp1;
      }else{
        LOGE(LOG_TAG,"TEMP_SETPOINT_SET_TOPIC publish error");
      }
    }
  }
  
  // MODE 
  else if (!strcmp(topic, MODE_STATE_TOPIC)) {
    
    char * mode=getPayloadCharValue("mode",p);
    if (mode!=NULL){
      LOGD(LOG_TAG,"MODE_STATE_TOPIC getPayloadCharValue:[%s]",mode);
      if (!strcmp(mode,"heat")){
        bHeatingMode=true;
        LOGD(LOG_TAG,"MODE_STATE_TOPIC mode:[%s] bCentralHeatingEnable:true",p);
      }else if (!strcmp(mode,"off")){
        bHeatingMode=false;
        LOGD(LOG_TAG,"MODE_STATE_TOPIC mode:[%s] bCentralHeatingEnable:false",p);
      }else{
        LOGE(LOG_TAG,"MODE_STATE_TOPIC mode:[%s] unknown",p);
      }
      free(mode);
    }else{
      LOGE(LOG_TAG,"MODE_STATE_TOPIC mode:[%s] is null",p);
    }
  }else if (!strcmp(topic, MODE_SET_TOPIC)) {
    if (!strcmp(p,"heat") && bCentralHeatingEnable==true){
      
      pubResult=publishToTopicStr(p,MODE_STATE_TOPIC,"mode",true); // Publish the new température;
      if (pubResult==true){
        bHeatingMode=true;
        LOGD(LOG_TAG,"MODE_SET_TOPIC mode:[%s] bCentralHeatingEnable:true",p);
      }else{
        LOGE(LOG_TAG,"MODE_SET_TOPIC publish error");
      }
    }else if (!strcmp(p,"off")){
      pubResult=publishToTopicStr(p,MODE_STATE_TOPIC,"mode",true); // Publish the new température;
      if (pubResult==true){
        bHeatingMode=false;
        LOGD(LOG_TAG,"MODE_SET_TOPIC mode:[%s] bCentralHeatingEnable:false",p);
      }else{
        LOGD(LOG_TAG,"MODE_SET_TOPIC publish error");
      }
    }else
      LOGE(LOG_TAG,"MODE_SET_TOPIC mode:[%s] unknown",p);
  }

  // TEMP_DWH
  else if (!strcmp(topic, TEMP_DHW_STATE_TOPIC)) {
    float dwhTarget1=getPayloadFloatValue("temp",p);
    if (dwhTarget1>0) {
      dwhTarget = dwhTarget1;
      bParamChanged=true;
      LOGD(LOG_TAG,"TEMP_DHW_STATE_TOPIC temperature:[%f]",dwhTarget1);
    }else{
      LOGE(LOG_TAG,"TEMP_DHW_STATE_TOPIC value <= 0 error");
    }
  }
  else if (!strcmp(topic, TEMP_DHW_SET_TOPIC)) {
    float dwhTarget1 = atof(p);
    if (!isnan(dwhTarget1) && isValidNumber(p)) {
      pubResult=publishToTopicFloat(dwhTarget1,TEMP_DHW_STATE_TOPIC,"temp");
      if (pubResult==true){
        dwhTarget = dwhTarget1;
        LOGD(LOG_TAG,"TEMP_DHW_SET_TOPIC temperature:[%f]",dwhTarget1);
      }else{
        LOGE(LOG_TAG,"TEMP_DHW_SET_TOPIC publish error");
      }
    }else{
      LOGE(LOG_TAG,"TEMP_DHW_SET_TOPIC value convert error");
    }
  }
    // OT_LOG
  else if (!strcmp(topic, ENABLE_OT_LOG_STATE_TOPIC)) {
    if (!strcmp(p, "1")){
      bOtLogEnable=true;
      bParamChanged=true;
      LOGI(LOG_TAG,"ENABLE_OT_LOG_STATE_TOPIC bOtLogEnable:[true]");
    }else if (!strcmp(p, "0")){
      bOtLogEnable=false;
      bParamChanged=true;
      LOGI(LOG_TAG,"ENABLE_OT_LOG_STATE_TOPIC bOtLogEnable:[false]");
    }else{
      LOGE(LOG_TAG,"ENABLE_OT_LOG_STATE_TOPIC unknow param value error");
    }
  }
// OTLOG
  else if (!strcmp(topic, ENABLE_OT_LOG_SET_TOPIC)) {
    if (!strcmp(p, "1")){
      pubResult=client.publish(ENABLE_OT_LOG_STATE_TOPIC,(const unsigned char *) p, length,true);
      if (pubResult==true){
        bOtLogEnable=true;//
        bParamChanged=true;
        LOGI(LOG_TAG,"ENABLE_OT_LOG_SET_TOPIC bOtLogEnable:[true]");
      }else{
        LOGE(LOG_TAG,"ENABLE_OT_LOG_SET_TOPIC publish error");
      }
    }else if (!strcmp(p, "0")){
       pubResult=client.publish(ENABLE_OT_LOG_STATE_TOPIC,(const unsigned char *) p, length,true);
      if (pubResult==true){
        bOtLogEnable=false;
        bParamChanged=true;
        LOGI(LOG_TAG,"ENABLE_OT_LOG_SET_TOPIC bOtLogEnable:[false]");
      }else{
        LOGE(LOG_TAG,"ENABLE_OT_LOG_SET_TOPIC publish error");
      }
    }else{
      LOGE(LOG_TAG,"ENABLE_OT_LOG_SET_TOPIC unknow param value error");
    }
  }


   else if (!strcmp(topic, ENABLE_EXTTEMP_STATE_TOPIC)) {  // <--- TO BE TESTED
    if (!strcmp(p, "1")){
      bExtTempEnable=true;
      bParamChanged=true;
      LOGI(LOG_TAG,"ENABLE_EXTTEMP_STATE_TOPIC bExtTempEnable:[true]");
    }else if (!strcmp(p, "0")){
      bExtTempEnable=false;
      bParamChanged=true;
      LOGI(LOG_TAG,"ENABLE_EXTTEMP_STATE_TOPIC bExtTempEnable:[false]");
    }else{
      LOGE(LOG_TAG,"ENABLE_EXTTEMP_STATE_TOPIC unknow param value error");
    }
  }

  else if (!strcmp(topic, ENABLE_EXTTEMP_SET_TOPIC)) {
    if (!strcmp(p, "1")){
      pubResult=client.publish(ENABLE_EXTTEMP_STATE_TOPIC,(const unsigned char *) p, length,true);
      if (pubResult==true){
        //bExtTempEnable=true;//
        //bParamChanged=true;
        LOGI(LOG_TAG,"ENABLE_EXTTEMP_SET_TOPIC bExtTempEnable:[true]");
      }else{
        LOGE(LOG_TAG,"ENABLE_EXTTEMP_SET_TOPIC publish error");
      }
    }else if (!strcmp(p, "0")){
       pubResult=client.publish(ENABLE_EXTTEMP_STATE_TOPIC,(const unsigned char *) p, length,true);
      if (pubResult==true){
        //bExtTempEnable=false;
        //bParamChanged=true;
        LOGI(LOG_TAG,"ENABLE_EXTTEMP_SET_TOPIC bExtTempEnable:[false]");
      }else{
        LOGE(LOG_TAG,"ENABLE_EXTTEMP_SET_TOPIC publish error");
      }
    }else{
      LOGE(LOG_TAG,"ENABLE_EXTTEMP_SET_TOPIC unknow param value error");
    }
  }
  // SW_CHEATING 
    else if (!strcmp(topic, SW_CHEATING_STATE_TOPIC)) {
    if (!strcmp(p, "1")){
      bCentralHeatingSw=true;
      bParamChanged=true;
      LOGI(LOG_TAG,"SW_CHEATING_STATE_TOPIC bCentralHeatingEnable:[true]");
    }else if (!strcmp(p, "0")){
      bCentralHeatingSw=false;
      bParamChanged=true;
      LOGI(LOG_TAG,"SW_CHEATING_STATE_TOPIC bCentralHeatingEnable:[false]");
    }else{
      LOGE(LOG_TAG,"SW_CHEATING_STATE_TOPIC unknow param value error");
    }
  }
  else if (!strcmp(topic, SW_CHEATING_SET_TOPIC)) {
    if (!strcmp(p, "1")){
      pubResult=client.publish(SW_CHEATING_STATE_TOPIC,(const unsigned char *) p, length,true);
      if (pubResult==true){
       
        LOGI(LOG_TAG,"SW_CHEATING_SET_TOPIC bCentralHeatingEnable:[true]");
      }else{
        LOGE(LOG_TAG,"SW_CHEATING_SET_TOPIC publish error");
      }
    }else if (!strcmp(p, "0")){
       pubResult=client.publish(SW_CHEATING_STATE_TOPIC,(const unsigned char *) p, length,true);
      if (pubResult==true){
        LOGI(LOG_TAG,"SW_CHEATING_SET_TOPIC bCentralHeatingEnable:[false]");
      }else{
        LOGE(LOG_TAG,"SW_CHEATING_SET_TOPIC publish error");
      }
    }else{
      LOGE(LOG_TAG,"SW_CHEATING_SET_TOPIC unknow param value error");
    }
  }
  // ENABLE CHEATING 
  else if (!strcmp(topic, ENABLE_CHEATING_STATE_TOPIC)) {
    if (!strcmp(p, "1")){
      bCentralHeatingEnable=true;
      bParamChanged=true;
      LOGI(LOG_TAG,"ENABLE_CHEATING_STATE_TOPIC bCentralHeatingEnable:[true]");
    }else if (!strcmp(p, "0")){
      bCentralHeatingEnable=false;
      bParamChanged=true;
      LOGI(LOG_TAG,"ENABLE_CHEATING_STATE_TOPIC bCentralHeatingEnable:[false]");
    }else{
      LOGE(LOG_TAG,"ENABLE_CHEATING_STATE_TOPIC unknow param value error");
    }
  }
  else if (!strcmp(topic, ENABLE_CHEATING_SET_TOPIC)) {
    if (!strcmp(p, "1")){
      pubResult=client.publish(ENABLE_CHEATING_STATE_TOPIC,(const unsigned char *) p, length,true);
      if (pubResult==true){
        bCentralHeatingEnable=true;
        bParamChanged=true;
        LOGI(LOG_TAG,"ENABLE_CHEATING_SET_TOPIC bCentralHeatingEnable:[true]");
      }else{
        LOGE(LOG_TAG,"ENABLE_CHEATING_SET_TOPIC publish error");
      }
    }else if (!strcmp(p, "0")){
       pubResult=client.publish(ENABLE_CHEATING_STATE_TOPIC,(const unsigned char *) p, length,true);
      if (pubResult==true){
        bCentralHeatingEnable=false;
        bParamChanged=true;
        LOGI(LOG_TAG,"ENABLE_CHEATING_SET_TOPIC bCentralHeatingEnable:[false]");
      }else{
        LOGE(LOG_TAG,"ENABLE_CHEATING_SET_TOPIC publish error");
      }
    }else{
      LOGE(LOG_TAG,"ENABLE_CHEATING_SET_TOPIC unknow param value error");
    }
  }

  // ENABLE WHEATING 
  else if (!strcmp(topic, ENABLE_WHEATING_STATE_TOPIC)) {
    if (!strcmp(p, "1")){
      bWaterHeatingEnable=true;
      bParamChanged=true;
      LOGI(LOG_TAG,"ENABLE_WHEATING_STATE_TOPIC bWaterHeatingEnable:[true]");
    }else if (!strcmp(p, "0")){
      bWaterHeatingEnable=false;
      bParamChanged=true;
      LOGI(LOG_TAG,"ENABLE_WHEATING_STATE_TOPIC bWaterHeatingEnable:[false]");
    }else{
      LOGE(LOG_TAG,"ENABLE_WHEATING_STATE_TOPIC unknow param value error");
    }
  }
  else if (!strcmp(topic, ENABLE_WHEATING_SET_TOPIC)) {
    if (!strcmp(p, "1")){
      pubResult=client.publish(ENABLE_WHEATING_STATE_TOPIC,(const unsigned char *) p, length,true);
      if (pubResult==true){
        bWaterHeatingEnable=true;
        LOGI(LOG_TAG,"ENABLE_WHEATING_SET_TOPIC bWaterHeatingEnable:[true]");
      }else{
        LOGE(LOG_TAG,"ENABLE_WHEATING_SET_TOPIC publish error");
      }
    }else if (!strcmp(p, "0")){
      pubResult=client.publish(ENABLE_WHEATING_STATE_TOPIC, (const unsigned char *) p, length,true);
      if (pubResult==true){
        bWaterHeatingEnable=false;
        LOGI(LOG_TAG,"ENABLE_WHEATING_SET_TOPIC bWaterHeatingEnable:[false]");
      }else{
         LOGE(LOG_TAG,"ENABLE_WHEATING_SET_TOPIC publish error");
      }
    }else{
      LOGE(LOG_TAG,"ENABLE_WHEATING_SET_TOPIC unknow param value error");
    }
  }

  //MAX_MODULATION_LEVEL
  else if (!strcmp(topic, MAX_MODULATION_LEVEL_STATE_TOPIC)) {
    int MaxModLevel1=(int)getPayloadFloatValue("level",p);
    if (MaxModLevel1>0) {
      MaxModLevel = MaxModLevel1;
      bParamChanged=true;
      LOGI(LOG_TAG,"MAX_MODULATION_LEVEL_STATE_TOPIC MaxModLevel:[%d]",MaxModLevel1);
    }else{
      LOGE(LOG_TAG,"MAX_MODULATION_LEVEL_STATE_TOPIC value <= 0 error");
    }

  }
  else if (!strcmp(topic, MAX_MODULATION_LEVEL_SET_TOPIC)) {
    int MaxModLevel1 = atoi(p);
    if (!isnan(MaxModLevel1) && isValidNumber(p)) {
      pubResult=publishToTopicFloat(MaxModLevel1,MAX_MODULATION_LEVEL_STATE_TOPIC,"level",true);
      if (pubResult==true){
        MaxModLevel = MaxModLevel1;
        LOGI(LOG_TAG,"MAX_MODULATION_LEVEL_SET_TOPIC MaxModLevel:[%d]",MaxModLevel);
      }
    }else{
      LOGE(LOG_TAG,"MAX_MODULATION_LEVEL_SET_TOPIC value convert error");
    }

  }

  //LBAND_TEMP_STATE // OK
  else if (!strcmp(topic, LBAND_TEMP_STATE_TOPIC)) {
    int oplo1=getPayloadFloatValue("temp",p);
    if (oplo1>0 && oplo1<ophi) {
      oplo = oplo1;
      LOGI(LOG_TAG,"LBAND_TEMP_STATE_TOPIC low band:[%d]",oplo1);
    }else{
      LOGE(LOG_TAG,"LBAND_TEMP_STATE_TOPIC value <= 0 or >ophi error");
    }
  }
  else if (!strcmp(topic, LBAND_TEMP_SET_TOPIC)) {
    int oplo1 = atoi(p);
    if (!isnan(oplo1) && isValidNumber(p) && oplo1<ophi) {
      pubResult=publishToTopicFloat(oplo1,LBAND_TEMP_STATE_TOPIC,"temp",true);
      if (pubResult==true ){
        LOGI(LOG_TAG,"LBAND_TEMP_SET_TOPIC low band:[%d]",oplo1);
        oplo = oplo1;
      }
     
      
    }else{
      LOGE(LOG_TAG,"LBAND_TEMP_SET_TOPIC value convert error");
    }

  }

  //HBAND_TEMP_STATE // OK
  else if (!strcmp(topic, HBAND_TEMP_STATE_TOPIC)) {
    int ophi1=getPayloadFloatValue("temp",p);
    if (ophi1>0 && ophi1>oplo) {
      ophi = ophi1;
      LOGI(LOG_TAG,"HBAND_TEMP_STATE_TOPIC high band:[%d]",ophi1);
    }else{
      LOGE(LOG_TAG,"HBAND_TEMP_STATE_TOPIC value <= 0 or <oplo error");
    }
  }
  else if (!strcmp(topic, HBAND_TEMP_SET_TOPIC)) {
    float ophi1 = atof(p);
    if (!isnan(ophi1) && isValidNumber(p) && ophi1>oplo) {
      pubResult=publishToTopicFloat(ophi1,HBAND_TEMP_STATE_TOPIC,"temp",true);
      if (pubResult==true){
        ophi = ophi1;
        LOGI(LOG_TAG,"HBAND_TEMP_SET_TOPIC high band:[%d]",ophi);
      }
    }else{
      LOGE(LOG_TAG,"HBAND_TEMP_SET_TOPIC high band error");
    }
  }

  //NOSP_OVERRIDE_TEMP_STATE_TOPIC //OK
  else if (!strcmp(topic, NOSP_OVERRIDE_TEMP_STATE_TOPIC)) {
    float nosp_override1=getPayloadFloatValue("temp",p);
    if (nosp_override1>0) {
      nosp_override = nosp_override1;
      LOGI(LOG_TAG,"NOSP_OVERRIDE_TEMP_STATE_TOPIC no sp temp overrice:[%f]",nosp_override);
    }else{
      LOGE(LOG_TAG,"NOSP_OVERRIDE_TEMP_STATE_TOPIC value <= 0 error");
    }
  }
  else if (!strcmp(topic, NOSP_OVERRIDE_TEMP_SET_TOPIC)) {
    float nosp_override1 =atof(p);
    if (!isnan(nosp_override1) && isValidNumber(p)) {
      publishToTopicFloat(nosp_override1,NOSP_OVERRIDE_TEMP_STATE_TOPIC,"temp");
      nosp_override = nosp_override1;
      LOGI(LOG_TAG,"NOSP_OVERRIDE_TEMP_SET_TOPIC no sp temp overrice:[%f]",nosp_override);
    }else{
      LOGE(LOG_TAG,"NOSP_OVERRIDE_TEMP_SET_TOPIC value convert error");
    }
  }else if (!strcmp(topic,INIT_DEFAULT_VALUES_TOPIC)){
    publishInitializationValues();
  }
  else if (!strcmp(topic,TRIGGER_CYCLE_TOPIC)){
    bForceCycle=true;
  }



  free(p);
  return;
}

bool isValidNumber(char payload[]){
    bool isNumber = true;
    for(int i=0; i < strlen(payload); i++){
        if(!isdigit(payload[i]) && payload[i]!='.'){
            isNumber = false;
            break;
        }
    } 
    return isNumber;
}


float getPayloadFloatValue(const char * key, char * payload){
  

  StaticJsonDocument<256> doc;
  DeserializationError err =deserializeJson(doc, payload);
  float res=-1;
  if (err) {
    LOGE(LOG_TAG,"deserializeJson() failed:%s payload:%s returning",err.c_str(),payload);
    return -1;
  }
  
  LOGD(LOG_TAG,"key:[%s], payload:[%s]",key,payload);

  if (doc.containsKey(key)){
    res=doc[key];
    LOGI(LOG_TAG,"getPayloadFloatValue: key:[%s] value:[%f]",key,res);
    return res;       
  }else{
    LOGE(LOG_TAG,"getPayloadFloatValue does nto contain key:[%s]",key);
    return -1;
  }
}

char * getPayloadCharValue(const char * key, char * payload){
  StaticJsonDocument<256> doc;
  DeserializationError err =deserializeJson(doc, payload);
   
  if (err) {
    LOGI(LOG_TAG,"deserializeJson() failed: %s",err.c_str());
  }

  LOGI(LOG_TAG,"key:[%s]",key);
  if (doc.containsKey(key)){
    const char * tt=doc[key];
    int i=strlen(tt)+1;
    char * res=(char *)malloc(i*sizeof(char));

    LOGI(LOG_TAG,"getPayloadCharValue:[%s]",tt);
    
    sprintf(res,"%s",tt);
    return res;       // <------ TODO PROPER FREE
  }else
    return NULL;  
}

bool publishToTopicFloat(float value,const char *topic,const char * key,bool retain){
 
  char mqttPayload[256];
  sprintf(mqttPayload,"{\"%s\":%f}",key,value);
  size_t n=strlen(mqttPayload);

  bool published=client.publish(topic,(const unsigned char*)mqttPayload,n,true);
 
  return published;
}

bool publishToTopicStr(char * value,const char *topic,const char * key,bool retain){
  
  char mqttPayload[256];

  sprintf(mqttPayload,"{\"%s\":\"%s\"}",key,value);
  size_t n=strlen(mqttPayload);

  bool published=client.publish(topic,(const unsigned char*) mqttPayload, n,retain);
 
  return published;
}

void publishInitializationValues(){

  char mqttPayload[32];
  bool published;
  size_t n=0;
  LOGD("MAIN","Init default Values");
  bool pubResult;

  pubResult=publishToTopicFloat(INITIAL_TEMP,CURRENT_TEMP_STATE_TOPIC,"temp",true); 
  pubResult=publishToTopicFloat(INITIAL_SP,TEMP_SETPOINT_STATE_TOPIC,"temp",true); 
  pubResult=publishToTopicFloat(LOW_BAND_TEMP,LBAND_TEMP_STATE_TOPIC,"temp",true); 
  pubResult=publishToTopicFloat(HIGH_BAND_TEMP,HBAND_TEMP_STATE_TOPIC,"temp",true); 
  pubResult=publishToTopicFloat(MAX_MODULATION_LEVEL,MAX_MODULATION_LEVEL_STATE_TOPIC,"level",true); 
  pubResult=publishToTopicFloat(INITIAL_TARGET_DWH_TEMP,TEMP_DHW_STATE_TOPIC,"temp",true); 
  pubResult=publishToTopicFloat(INITIAL_NO_SP_TEMP_OVERRIDE,NOSP_OVERRIDE_TEMP_STATE_TOPIC,"temp",true); 
  pubResult=publishToTopicFloat(0,CURRENT_EXTEMP_STATE_TOPIC,"temp",true); 
  pubResult=publishToTopicFloat(Kp,PID_KP_STATE_TOPIC,"value",true); 
  pubResult=publishToTopicFloat(Ki,PID_KI_STATE_TOPIC,"value",true); 
  pubResult=publishToTopicFloat(Kd,PID_KD_STATE_TOPIC,"value",true); 
  pubResult=publishToTopicFloat(Ke,PID_KE_STATE_TOPIC,"value",true); 
  pubResult=publishToTopicFloat(pid_interval,PID_INTERVAL_STATE_TOPIC,"value",true); 

  sprintf(mqttPayload,"0");
  n=strlen(mqttPayload);
  published=client.publish(ENABLE_CHEATING_SET_TOPIC, mqttPayload, n);

  sprintf(mqttPayload,"0");
  n=strlen(mqttPayload);
  published=client.publish(ENABLE_WHEATING_SET_TOPIC, mqttPayload, n);

  sprintf(mqttPayload,"0");
  n=strlen(mqttPayload);
  published=client.publish(ENABLE_OT_LOG_SET_TOPIC, mqttPayload, n);

  sprintf(mqttPayload,"Init Default control values");
  publishToTopicStr(mqttPayload,OT_LOG_STATE_TOPIC,"text",false);

  sprintf(mqttPayload,"0");
  n=strlen(mqttPayload);
  published=client.publish(ENABLE_EXTTEMP_SET_TOPIC, mqttPayload, n);

}

void publishAvailable(){

  char mqttPayload[256];
  bool published;
 
  sprintf(mqttPayload,"ONLINE");
  size_t n=strlen(mqttPayload);
  published=client.publish(AVAILABILITY_TOPIC,(const unsigned char*)mqttPayload , n,true);
 
}


