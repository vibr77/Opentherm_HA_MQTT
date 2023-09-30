

#include <Arduino.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>

#include "include/mqtt_com.h"
#include "esp_log.h"

extern PubSubClient client;

extern float oplo,ophi,sp,t,ierr,op;
extern bool bCentralHeating, bWaterHeatingEnable,bCentralHeatingEnable,bHeatingMode;
extern bool bWaterHeating;
extern float dwhTarget;
extern float dwhTemp;
extern int MaxModLevel;
extern float nosp_override;
extern bool bParamChanged;
extern float flameLevel;
extern unsigned long lastSpSet,lastPSet;

#define LOG_TAG "mqtt"

void LogMasterParam(){

  ESP_LOGI(LOG_TAG,"Master Param oplo:[%f]",oplo);
  ESP_LOGI(LOG_TAG,"Master Param ophi:[%f]",ophi);
  ESP_LOGI(LOG_TAG,"Master Param sp:[%f]",sp);
  ESP_LOGI(LOG_TAG,"Master Param t:[%f]",t);
  ESP_LOGI(LOG_TAG,"Master Param ierr:[%f]",ierr);
  ESP_LOGI(LOG_TAG,"Master Param op:[%f]",op);
  ESP_LOGI(LOG_TAG,"Master Param nosp:[%f]",nosp_override);
  ESP_LOGI(LOG_TAG,"Master Param MaxModLevel:[%d]",MaxModLevel);
  ESP_LOGI(LOG_TAG,"Master Param dwhTarget:[%f]",dwhTarget);
  ESP_LOGI(LOG_TAG,"Master Param dwhTemp:[%f]",dwhTemp);
  ESP_LOGI(LOG_TAG,"Master Param FlameLevel:[%f]",flameLevel);

  if (bWaterHeatingEnable==true)
    ESP_LOGI(LOG_TAG,"Master Param bWaterHeatingEnable:[true]");
  else
    ESP_LOGI(LOG_TAG,"Master Param bWaterHeatingEnable:[false]");

  if (bCentralHeatingEnable==true)
    ESP_LOGI(LOG_TAG,"Master Param bCentralHeatingEnable:[true]");
  else
    ESP_LOGI(LOG_TAG,"Master Param bCentralHeatingEnable:[false]");
  
  if (bHeatingMode==true)
    ESP_LOGI(LOG_TAG,"Master Param bHeatingMode:[true]");
  else
    ESP_LOGI(LOG_TAG,"Master Param bHeatingMode:[false]");

}


DynamicJsonDocument getDeviceBlock(){
    
    DynamicJsonDocument doc(1024);
    
    doc["dev"]["ids"][0]=MQTT_DEVICENAME;
    doc["dev"]["name"]="Boiler controller ";
    doc["dev"]["mdl"]="ESP32_MASTER_OT_01";
    doc["dev"]["mf"]="VIBR";
    doc["dev"]["sw"]=SW_VERSION;
    doc["dev"]["hw_version"]="1.0";

    doc["availability"]["topic"]=AVAILABILITY_TOPIC;
    doc["availability"]["payload_available"]="ONLINE";
    doc["availability"]["payload_not_available"]="OFFLINE";
    doc["availability"]["value_template"]="{{ value_json.status }}";

    return doc;
}

boolean sendMqttMsg(const char* topic,DynamicJsonDocument doc){
    String jsonBuffer;
    size_t n = serializeJson(doc, jsonBuffer);
    bool published=client.publish(topic, jsonBuffer.c_str(), n);
    return published;
}

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

void MQTT_DiscoveryMsg_Sensor_CentralHeating(){

  DynamicJsonDocument doc(2048);

  doc["name"] = "Central Heating";
  char ID[64];
  sprintf(ID,"%s_CENTRALH",MQTT_DEV_UNIQUE_ID);
  doc["uniq_id"]=ID;
  doc["icon"]="mdi:fire";
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

void MQTT_DiscoveryMsg_Sensor_WaterHeating(){

  DynamicJsonDocument doc(2048);

  doc["name"] = "DWH Heating";
  char ID[64];
  sprintf(ID,"%s_WATERH",MQTT_DEV_UNIQUE_ID);
  doc["uniq_id"]=ID;

  doc["icon"]="mdi:fire";
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

void MQTT_DiscoveryMsg_Sensor_IntegralError(){

  DynamicJsonDocument doc(2048);

  doc["name"] = "Boiler integral error";
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

void MQTT_DiscoveryMsg_Switch_EnableCentralHeating(){

  DynamicJsonDocument doc(2048);

  doc["name"] = "Central Heating";
  doc["icon"]="mdi:fire";
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
  doc["retain"]=true;
  doc["entity_category"]="diagnostic";
  doc["command_topic"]=INIT_DEFAULT_VALUES_TOPIC;
  
  DynamicJsonDocument dev=getDeviceBlock();
  doc["dev"]=dev["dev"];
  doc["availability"]=dev["availability"];

  bool published= sendMqttMsg(DISCOVERY_INIT_DEFAULT_VALUES_TOPIC,doc);

}

void callback(char* topic, byte* payload, unsigned int length) {
  ESP_LOGD(LOG_TAG,"MQTT Callback topic:[%s]",topic);

  char* p = (char*)malloc((length+1)*sizeof(char));
  memcpy(p,payload,length);
  p[length]=0;
  bool pubResult=false;

  ESP_LOGD(LOG_TAG,"MQTT Callback topic:[%s] payload:[%s]",topic,p);

  // CURRENT TEMPERATURE
  if (!strcmp(topic, CURRENT_TEMP_STATE_TOPIC)) {
    float t1=getPayloadFloatValue("temp",p);
    if (t1>0) {
      t=t1;
      ESP_LOGD(LOG_TAG,"CURRENT_TEMP_STATE_TOPIC new temperature:%f",t1);
      unsigned long now = millis();
      lastPSet=now;
    }else{
      ESP_LOGE(LOG_TAG,"CURRENT_TEMP_STATE_TOPIC value <= 0 error");
    }
  }
  else if (!strcmp(topic, CURRENT_TEMP_SET_TOPIC)) {
    float t1=atof(p);
    if (!isnan(t1) && isValidNumber(p)) {
      pubResult=publishToTopicFloat(t1,CURRENT_TEMP_STATE_TOPIC,"temp",true); // Publish the new température;
      if (pubResult==true){
        t=t1;
        ESP_LOGD(LOG_TAG,"CURRENT_TEMP_SET_TOPIC new temperature:%f",t1);
       
        
      }else{
        ESP_LOGE(LOG_TAG,"CURRENT_TEMP_SET_TOPIC publish error");
      }
    }
  }

  // SETPOINT
  else if (!strcmp(topic, TEMP_SETPOINT_STATE_TOPIC)) {
    float sp1=getPayloadFloatValue("temp",p);
    ESP_LOGD(LOG_TAG,"TEMP_SETPOINT_STATE_TOPIC payload:[%s] conversion to float:[%f]",p,sp1);
    if (sp1>0) {
      ESP_LOGD(LOG_TAG,"TEMP_SETPOINT_STATE_TOPIC new temperature:%f",sp1);
      sp=sp1;
      unsigned long now = millis();
      lastSpSet=now;
    }else{
      ESP_LOGE(LOG_TAG,"TEMP_SETPOINT_STATE_TOPIC value <= 0 error");
    }
  
  }else if (!strcmp(topic, TEMP_SETPOINT_SET_TOPIC)) {
    float sp1 = atof(p);
    if (!isnan(sp1) && isValidNumber(p)) {
      
      pubResult=publishToTopicFloat(sp1,TEMP_SETPOINT_STATE_TOPIC,"temp",true); // Publish the new température;
      if (pubResult==true){
        ESP_LOGD(LOG_TAG,"TEMP_SETPOINT_SET_TOPIC temperature:[%f]",sp1);
        sp=sp1;
      }else{
        ESP_LOGE(LOG_TAG,"TEMP_SETPOINT_SET_TOPIC publish error");
      }
    }
  }
  
  // MODE 
  else if (!strcmp(topic, MODE_STATE_TOPIC)) {
    
    char * mode=getPayloadCharValue("mode",p);
    if (mode!=NULL){
      ESP_LOGD(LOG_TAG,"MODE_STATE_TOPIC getPayloadCharValue:[%s]",mode);
      if (!strcmp(mode,"heat")){
        bHeatingMode=true;
        ESP_LOGD(LOG_TAG,"MODE_STATE_TOPIC mode:[%s] bCentralHeatingEnable:true",p);
      }else if (!strcmp(mode,"off")){
        bHeatingMode=false;
        ESP_LOGD(LOG_TAG,"MODE_STATE_TOPIC mode:[%s] bCentralHeatingEnable:false",p);
      }else{
        ESP_LOGE(LOG_TAG,"MODE_STATE_TOPIC mode:[%s] unknown",p);
      }
      free(mode);
    }else{
      ESP_LOGE(LOG_TAG,"MODE_STATE_TOPIC mode:[%s] is null",p);
    }
  }else if (!strcmp(topic, MODE_SET_TOPIC)) {
    if (!strcmp(p,"heat") && bCentralHeatingEnable==true){
      
      pubResult=publishToTopicStr(p,MODE_STATE_TOPIC,"mode",true); // Publish the new température;
      if (pubResult==true){
        bHeatingMode=true;
        ESP_LOGD(LOG_TAG,"MODE_SET_TOPIC mode:[%s] bCentralHeatingEnable:true",p);
      }else{
        ESP_LOGE(LOG_TAG,"MODE_SET_TOPIC publish error");
      }
    }else if (!strcmp(p,"off")){
      pubResult=publishToTopicStr(p,MODE_STATE_TOPIC,"mode",true); // Publish the new température;
      if (pubResult==true){
        bHeatingMode=false;
        ESP_LOGD(LOG_TAG,"MODE_SET_TOPIC mode:[%s] bCentralHeatingEnable:false",p);
      }else{
        ESP_LOGD(LOG_TAG,"MODE_SET_TOPIC publish error");
      }
    }else
      ESP_LOGE(LOG_TAG,"MODE_SET_TOPIC mode:[%s] unknown",p);
  }

  // TEMP_DWH
  else if (!strcmp(topic, TEMP_DHW_STATE_TOPIC)) {
    float dwhTarget1=getPayloadFloatValue("temp",p);
    if (dwhTarget1>0) {
      dwhTarget = dwhTarget1;
      bParamChanged=true;
      ESP_LOGD(LOG_TAG,"TEMP_DHW_STATE_TOPIC temperature:[%f]",dwhTarget1);
    }else{
      ESP_LOGE(LOG_TAG,"TEMP_DHW_STATE_TOPIC value <= 0 error");
    }
  }
  else if (!strcmp(topic, TEMP_DHW_SET_TOPIC)) {
    float dwhTarget1 = atof(p);
    if (!isnan(dwhTarget1) && isValidNumber(p)) {
      pubResult=publishToTopicFloat(dwhTarget1,TEMP_DHW_STATE_TOPIC,"temp");
      if (pubResult==true){
        dwhTarget = dwhTarget1;
        ESP_LOGD(LOG_TAG,"TEMP_DHW_SET_TOPIC temperature:[%f]",dwhTarget1);
      }else{
        ESP_LOGE(LOG_TAG,"TEMP_DHW_SET_TOPIC publish error");
      }
    }else{
      ESP_LOGE(LOG_TAG,"TEMP_DHW_SET_TOPIC value convert error");
    }
  }

  // ENABLE CHEATING 
  else if (!strcmp(topic, ENABLE_CHEATING_STATE_TOPIC)) {
    if (!strcmp(p, "1")){
      bCentralHeatingEnable=true;
      bParamChanged=true;
      ESP_LOGI(LOG_TAG,"ENABLE_CHEATING_STATE_TOPIC bCentralHeatingEnable:[true]");
    }else if (!strcmp(p, "0")){
      bCentralHeatingEnable=false;
      bParamChanged=true;
      ESP_LOGI(LOG_TAG,"ENABLE_CHEATING_STATE_TOPIC bCentralHeatingEnable:[false]");
    }else{
      ESP_LOGE(LOG_TAG,"ENABLE_CHEATING_STATE_TOPIC unknow param value error");
    }
  }
  else if (!strcmp(topic, ENABLE_CHEATING_SET_TOPIC)) {
    if (!strcmp(p, "1")){
      pubResult=client.publish(ENABLE_CHEATING_STATE_TOPIC, p, length,true);
      if (pubResult==true){
        bCentralHeatingEnable=true;
        bParamChanged=true;
        ESP_LOGI(LOG_TAG,"ENABLE_CHEATING_SET_TOPIC bCentralHeatingEnable:[true]");
      }else{
        ESP_LOGE(LOG_TAG,"ENABLE_CHEATING_SET_TOPIC publish error");
      }
    }else if (!strcmp(p, "0")){
       pubResult=client.publish(ENABLE_CHEATING_STATE_TOPIC, p, length,true);
      if (pubResult==true){
        bCentralHeatingEnable=false;
        bParamChanged=true;
        ESP_LOGI(LOG_TAG,"ENABLE_CHEATING_SET_TOPIC bCentralHeatingEnable:[false]");
      }else{
        ESP_LOGE(LOG_TAG,"ENABLE_CHEATING_SET_TOPIC publish error");
      }
    }else{
      ESP_LOGE(LOG_TAG,"ENABLE_CHEATING_SET_TOPIC unknow param value error");
    }
  }

  // ENABLE WHEATING 
  else if (!strcmp(topic, ENABLE_WHEATING_STATE_TOPIC)) {
    if (!strcmp(p, "1")){
      bWaterHeatingEnable=true;
      bParamChanged=true;
      ESP_LOGI(LOG_TAG,"ENABLE_WHEATING_STATE_TOPIC bWaterHeatingEnable:[true]");
    }else if (!strcmp(p, "0")){
      bWaterHeatingEnable=false;
      bParamChanged=true;
      ESP_LOGI(LOG_TAG,"ENABLE_WHEATING_STATE_TOPIC bWaterHeatingEnable:[false]");
    }else{
      ESP_LOGE(LOG_TAG,"ENABLE_WHEATING_STATE_TOPIC unknow param value error");
    }
  }
  else if (!strcmp(topic, ENABLE_WHEATING_SET_TOPIC)) {
    if (!strcmp(p, "1")){
      pubResult=client.publish(ENABLE_WHEATING_STATE_TOPIC, p, length,true);
      if (pubResult==true){
        bWaterHeatingEnable=true;
        ESP_LOGI(LOG_TAG,"ENABLE_WHEATING_SET_TOPIC bWaterHeatingEnable:[true]");
      }else{
        ESP_LOGE(LOG_TAG,"ENABLE_WHEATING_SET_TOPIC publish error");
      }
    }else if (!strcmp(p, "0")){
      pubResult=client.publish(ENABLE_WHEATING_STATE_TOPIC, p, length,true);
      if (pubResult==true){
        bWaterHeatingEnable=false;
        ESP_LOGI(LOG_TAG,"ENABLE_WHEATING_SET_TOPIC bWaterHeatingEnable:[false]");
      }else{
         ESP_LOGE(LOG_TAG,"ENABLE_WHEATING_SET_TOPIC publish error");
      }
    }else{
      ESP_LOGE(LOG_TAG,"ENABLE_WHEATING_SET_TOPIC unknow param value error");
    }
  }

  //MAX_MODULATION_LEVEL
  else if (!strcmp(topic, MAX_MODULATION_LEVEL_STATE_TOPIC)) {
    int MaxModLevel1=(int)getPayloadFloatValue("level",p);
    if (MaxModLevel1>0) {
      MaxModLevel = MaxModLevel1;
      bParamChanged=true;
      ESP_LOGI(LOG_TAG,"MAX_MODULATION_LEVEL_STATE_TOPIC MaxModLevel:[%d]",MaxModLevel1);
    }else{
      ESP_LOGE(LOG_TAG,"MAX_MODULATION_LEVEL_STATE_TOPIC value <= 0 error");
    }

  }
  else if (!strcmp(topic, MAX_MODULATION_LEVEL_SET_TOPIC)) {
    int MaxModLevel1 = atoi(p);
    if (!isnan(MaxModLevel1) && isValidNumber(p)) {
      pubResult=publishToTopicFloat(MaxModLevel1,MAX_MODULATION_LEVEL_STATE_TOPIC,"level",true);
      if (pubResult==true){
        MaxModLevel = MaxModLevel1;
        ESP_LOGI(LOG_TAG,"MAX_MODULATION_LEVEL_SET_TOPIC MaxModLevel:[%d]",MaxModLevel);
      }
    }else{
      ESP_LOGE(LOG_TAG,"MAX_MODULATION_LEVEL_SET_TOPIC value convert error");
    }

  }

  //LBAND_TEMP_STATE // OK
  else if (!strcmp(topic, LBAND_TEMP_STATE_TOPIC)) {
    int oplo1=getPayloadFloatValue("temp",p);
    if (oplo1>0 && oplo1<ophi) {
      oplo = oplo1;
      ESP_LOGI(LOG_TAG,"LBAND_TEMP_STATE_TOPIC low band:[%d]",oplo1);
    }else{
      ESP_LOGE(LOG_TAG,"LBAND_TEMP_STATE_TOPIC value <= 0 or >ophi error");
    }
  }
  else if (!strcmp(topic, LBAND_TEMP_SET_TOPIC)) {
    int oplo1 = atoi(p);
    if (!isnan(oplo1) && isValidNumber(p) && oplo1<ophi) {
      pubResult=publishToTopicFloat(oplo1,LBAND_TEMP_STATE_TOPIC,"temp",true);
      if (pubResult==true ){
        ESP_LOGI(LOG_TAG,"LBAND_TEMP_SET_TOPIC low band:[%d]",oplo1);
        oplo = oplo1;
      }
     
      
    }else{
      ESP_LOGE(LOG_TAG,"LBAND_TEMP_SET_TOPIC value convert error");
    }

  }

  //HBAND_TEMP_STATE // OK
  else if (!strcmp(topic, HBAND_TEMP_STATE_TOPIC)) {
    int ophi1=getPayloadFloatValue("temp",p);
    if (ophi1>0 && ophi1>oplo) {
      ophi = ophi1;
      ESP_LOGI(LOG_TAG,"HBAND_TEMP_STATE_TOPIC high band:[%d]",ophi1);
    }else{
      ESP_LOGE(LOG_TAG,"HBAND_TEMP_STATE_TOPIC value <= 0 or <oplo error");
    }
  }
  else if (!strcmp(topic, HBAND_TEMP_SET_TOPIC)) {
    float ophi1 = atof(p);
    if (!isnan(ophi1) && isValidNumber(p) && ophi1>oplo) {
      pubResult=publishToTopicFloat(ophi1,HBAND_TEMP_STATE_TOPIC,"temp",true);
      if (pubResult==true){
        ophi = ophi1;
        ESP_LOGI(LOG_TAG,"HBAND_TEMP_SET_TOPIC high band:[%d]",ophi);
      }
    }else{
      ESP_LOGE(LOG_TAG,"HBAND_TEMP_SET_TOPIC high band error");
    }
  }

  //NOSP_OVERRIDE_TEMP_STATE_TOPIC //OK
  else if (!strcmp(topic, NOSP_OVERRIDE_TEMP_STATE_TOPIC)) {
    float nosp_override1=getPayloadFloatValue("temp",p);
    if (nosp_override1>0) {
      nosp_override = nosp_override1;
      ESP_LOGI(LOG_TAG,"NOSP_OVERRIDE_TEMP_STATE_TOPIC no sp temp overrice:[%f]",nosp_override);
    }else{
      ESP_LOGE(LOG_TAG,"NOSP_OVERRIDE_TEMP_STATE_TOPIC value <= 0 error");
    }
  }
  else if (!strcmp(topic, NOSP_OVERRIDE_TEMP_SET_TOPIC)) {
    float nosp_override1 =atof(p);
    if (!isnan(nosp_override1) && isValidNumber(p)) {
      publishToTopicFloat(nosp_override1,NOSP_OVERRIDE_TEMP_STATE_TOPIC,"temp");
      nosp_override = nosp_override1;
      ESP_LOGI(LOG_TAG,"NOSP_OVERRIDE_TEMP_SET_TOPIC no sp temp overrice:[%f]",nosp_override);
    }else{
      ESP_LOGE(LOG_TAG,"NOSP_OVERRIDE_TEMP_SET_TOPIC value convert error");
    }
  }else if (!strcmp(topic,INIT_DEFAULT_VALUES_TOPIC)){
    publishInitializationValues();
  }
  free(p);
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
  DynamicJsonDocument doc(256);
  deserializeJson(doc, payload);
  float res= doc[key];
  return res;
}

char * getPayloadCharValue(const char * key, char * payload){
  StaticJsonDocument<256> doc;
  DeserializationError err =deserializeJson(doc, payload);
   
  if (err) {
    ESP_LOGI(LOG_TAG,"deserializeJson() failed: %s",err.c_str());
  }

  ESP_LOGI(LOG_TAG,"key:[%s]",key);
  if (doc.containsKey(key)){
    const char * tt=doc[key];
    int i=strlen(tt)+1;
    char * res=(char *)malloc(i*sizeof(char));

    ESP_LOGI(LOG_TAG,"getPayloadCharValue:[%s]",tt);
    
    sprintf(res,"%s",tt);
    return res;       // <------ TODO PROPER FREE
  }else
    return NULL;  
}

bool publishToTopicFloat(float value,const char *topic,const char * key,bool retain){
 
  char mqttPayload[256];
  sprintf(mqttPayload,"{\"%s\":%f}",key,value);
  size_t n=strlen(mqttPayload);

  bool published=client.publish(topic,mqttPayload,n,true);
 
  return published;
}

bool publishToTopicStr(char * value,const char *topic,const char * key,bool retain){
  
  char mqttPayload[256];

  sprintf(mqttPayload,"{\"%s\":\"%s\"}",key,value);
  size_t n=strlen(mqttPayload);

  bool published=client.publish(topic, mqttPayload, n,retain);
 
  return published;
}

void publishInitializationValues(){
  // 
  char mqttPayload[256];
  bool published;
  size_t n=0;

  sprintf(mqttPayload,"{\"temp\",%f}",INITIAL_TEMP);
  n=strlen(mqttPayload);
  published=client.publish(CURRENT_TEMP_STATE_TOPIC, mqttPayload, n);

  sprintf(mqttPayload,"{\"temp\",%f}",INITIAL_SP);
  n=strlen(mqttPayload);
  published=client.publish(TEMP_SETPOINT_STATE_TOPIC, mqttPayload, n);

  sprintf(mqttPayload,"{\"temp\",%f}",LOW_BAND_TEMP);
  n=strlen(mqttPayload);
  published=client.publish(LBAND_TEMP_STATE_TOPIC, mqttPayload, n);

  sprintf(mqttPayload,"{\"temp\",%f}",HIGH_BAND_TEMP);
  n=strlen(mqttPayload);
  published=client.publish(HBAND_TEMP_STATE_TOPIC, mqttPayload, n);

  sprintf(mqttPayload,"{\"level\",%f}",MAX_MODULATION_LEVEL);
  n=strlen(mqttPayload);
  published=client.publish(MAX_MODULATION_LEVEL_STATE_TOPIC, mqttPayload, n);

  sprintf(mqttPayload,"{\"temp\",%f}",INITIAL_TARGET_DWH_TEMP);
  n=strlen(mqttPayload);
  published=client.publish(TEMP_DHW_STATE_TOPIC, mqttPayload, n);

  sprintf(mqttPayload,"{\"temp\",%f}",INITIAL_NO_SP_TEMP_OVERRIDE);
  n=strlen(mqttPayload);
  published=client.publish(NOSP_OVERRIDE_TEMP_STATE_TOPIC, mqttPayload, n);

  sprintf(mqttPayload,"\"0\"");
  n=strlen(mqttPayload);
  published=client.publish(ENABLE_CHEATING_SET_TOPIC, mqttPayload, n);

  sprintf(mqttPayload,"\"0\"");
  n=strlen(mqttPayload);
  published=client.publish(ENABLE_WHEATING_SET_TOPIC, mqttPayload, n);

  sprintf(mqttPayload,"\"0\"");
  n=strlen(mqttPayload);
  published=client.publish(ENABLE_OT_LOG_SET_TOPIC, mqttPayload, n);

  sprintf(mqttPayload,"Init Default control values");
  publishToTopicStr(mqttPayload,OT_LOG_STATE_TOPIC,"text",false); 

}

void publishAvailable(){

  char mqttPayload[256];
  bool published;
  size_t n=0;

  sprintf(mqttPayload,"{\"status\",\"ONLINE\"}");
  published=client.publish(AVAILABILITY_TOPIC,mqttPayload , n,true);
 
}


