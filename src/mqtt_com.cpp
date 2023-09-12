

#include <Arduino.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>

#include "include/mqtt_com.h"

extern PubSubClient client;

DynamicJsonDocument getDeviceBlock(){
    
    DynamicJsonDocument doc(1024);
    
    doc["dev"]["ids"][0]=MQTT_DEVICENAME;
    doc["dev"]["name"]="OpenTherm Boiler Master Thermostat";
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
    Serial.println(topic);
    Serial.println(jsonBuffer);
    return published;
}

void MQTT_DiscoveryMsg_Climate(){
  
  DynamicJsonDocument doc(4096);

  doc["name"] = "Boiler Thermostat";
  doc["dev_cla"] = "climate";
  doc["uniq_id"]=MQTT_DEV_UNIQUE_ID+"_CLIMATE";;

  doc["precision"]=PRECISION;
  doc["initial"]=INITIAL_TEMP;
  doc["min_temp"]=MIN_SP_TEMP;
  doc["max_temp"]=MAX_SP_TEMP;
  doc["temperature_unit"]= TEMPERATURE_UNIT;
  doc["temp_step"]= TEMPERATURE_STEP;
  doc["optimistic"]=OPTIMISTIC;

  doc["current_temperature_topic"]=CURRENT_TEMP_STATE_TOPIC;
  doc["current_temperature_template"]="{{ value_json.temperature }}";
  
  doc["temperature_command_topic"]=TEMP_SETPOINT_SET_TOPIC;
  
  doc["temperature_state_topic"]=TEMP_SETPOINT_STATE_TOPIC;
  doc["temperature_state_template"]="{{ value_json.temperature }}";

  doc["mode_command_topic"]=MODE_SET_TOPIC;
  doc["mode_state_topic"]=MODE_STATE_TOPIC;
  doc["mode_state_template"]="{{ value_json.mode }}";
  
  doc["modes"][0]="heat";
  doc["modes"][1]="off";
 
  DynamicJsonDocument dev=getDeviceBlock();
  doc["dev"]=dev["dev"];
  doc["availability"]=dev["availability"];

  bool published= sendMqttMsg(DISCOVERY_CLIMATE_TOPIC.c_str(),doc);
}



void MQTT_DiscoveryMsg_Sensor_Flaming(){

  DynamicJsonDocument doc(2048);
 
  doc["name"] = "Boiler Flame";
  doc["dev_cla"] = "heat";
  doc["uniq_id"]=MQTT_DEV_UNIQUE_ID+"_FLAME";

  doc["payload_off"]="OFF";
  doc["payload_on"]="ON";
  doc["qos"]=0;
  doc["state_topic"]=FLAME_STATUS_GET_TOPIC;
  
  DynamicJsonDocument dev=getDeviceBlock();
  doc["dev"]=dev["dev"];
  doc["availability"]=dev["availability"];

  bool published= sendMqttMsg(DISCOVERY_FLAME_TOPIC.c_str(),doc);
}

void MQTT_DiscoveryMsg_Sensor_CentralHeating(){

  DynamicJsonDocument doc(2048);

  doc["name"] = "Central Heating";
  doc["dev_cla"] = "heat";
  doc["uniq_id"]=MQTT_DEV_UNIQUE_ID+"_CENTRALH";

  doc["payload_off"]="OFF";
  doc["payload_on"]="ON";
  doc["qos"]=0;
  doc["state_topic"]=CENTRAL_HEATING_STATE_TOPIC;
  
  DynamicJsonDocument dev=getDeviceBlock();
  doc["dev"]=dev["dev"];
  doc["availability"]=dev["availability"];

  bool published= sendMqttMsg(DISCOVERY_CENTRAL_HEATING_TOPIC.c_str(),doc);
}

void MQTT_DiscoveryMsg_Sensor_WaterHeating(){

  DynamicJsonDocument doc(2048);

  doc["name"] = "Water Heating";
  doc["dev_cla"] = "heat";
  doc["uniq_id"]=MQTT_DEV_UNIQUE_ID+"_WATERH";

  doc["payload_off"]="OFF";
  doc["payload_on"]="ON";
  doc["qos"]=0;
  doc["state_topic"]=WATER_HEATING_STATE_TOPIC;
  
  DynamicJsonDocument dev=getDeviceBlock();
  doc["dev"]=dev["dev"];
  doc["availability"]=dev["availability"];

  bool published= sendMqttMsg(DISCOVERY_WATER_HEATING_TOPIC.c_str(),doc);
}




void MQTT_DiscoveryMsg_Sensor_BoilerTemperature(){

  DynamicJsonDocument doc(4096);

  doc["name"] = "Boiler Temperature";
  doc["dev_cla"] = "temperature";
  doc["unit_of_measurement"] = "°C";
  
  doc["uniq_id"]=MQTT_DEV_UNIQUE_ID+"_BOILERTEMP";
  doc["qos"]=0;
  doc["state_topic"]=TEMP_BOILER_GET_TOPIC;
  
  DynamicJsonDocument dev=getDeviceBlock();
  doc["dev"]=dev["dev"];
  doc["availability"]=dev["availability"];

  bool published= sendMqttMsg(DISCOVERY_BOILER_TEMP_TOPIC.c_str(),doc);

}


void MQTT_DiscoveryMsg_Number_Modulation_Level(){

  DynamicJsonDocument doc(2048);

  doc["name"] = "Modulation Level";
  //doc["dev_cla"] = "number";
  doc["uniq_id"]=MQTT_DEV_UNIQUE_ID+"_MODLVL";
  doc["icon"]="mdi:sine-wave";

  doc["min"]=1;
  doc["max"]=100;
  doc["mode"]="slider";
  doc["step"]=1;
  doc["qos"]=2;
  doc["retain"]=true;
  doc["entity_category"]="config";
  doc["optimistic"]=false;
  doc["state_topic"]=MODULATION_LEVEL_STATE_TOPIC;
  doc["value_template"]="{{ value_json.level }}";
  
  doc["command_topic"]=MODULATION_LEVEL_SET_TOPIC;
  //doc["command_template"]="{{ value_json.mode }}";
  
  DynamicJsonDocument dev=getDeviceBlock();
  doc["dev"]=dev["dev"];
  doc["availability"]=dev["availability"];

  bool published= sendMqttMsg(DISCOVERY_MODLVL_TOPIC.c_str(),doc);
}

void MQTT_DiscoveryMsg_Switch_EnableCentralHeating(){

  DynamicJsonDocument doc(2048);

  doc["name"] = "Enable Central Heating";
  doc["icon"]="mdi:fire";
  
  doc["uniq_id"]=MQTT_DEV_UNIQUE_ID+"_ENABLE_CHEATING";
  doc["device_class"]="switch";
  
  doc["payload_off"]="0";
  doc["payload_on"]="1";

  doc["state_off"]="0";
  doc["state_on"]="1";
  
  doc["qos"]=0;
  doc["retain"]=true;
  doc["entity_category"]="config";
  doc["optimistic"]=false;

  doc["state_topic"]=ENABLE_CHEATING_STATE_TOPIC;
  doc["command_topic"]=ENABLE_CHEATING_SET_TOPIC;
  //doc["command_template"]="{{ value_json.state }}";
  
  DynamicJsonDocument dev=getDeviceBlock();
  doc["dev"]=dev["dev"];
  doc["availability"]=dev["availability"];

  bool published= sendMqttMsg(DISCOVERY_ENABLE_CHEATING_TOPIC.c_str(),doc);
}


void MQTT_DiscoveryMsg_Switch_EnableWaterHeating(){

  DynamicJsonDocument doc(2048);

  doc["name"] = "Enable Water Heating";
  doc["uniq_id"]=MQTT_DEV_UNIQUE_ID+"_ENABLE_WHEATING";
  doc["device_class"]="switch";
  doc["icon"]="mdi:fire";
  
  doc["payload_off"]="0";
  doc["payload_on"]="1";
 

  doc["state_off"]="0";
  doc["state_on"]="1";
  
  doc["qos"]=0;
  doc["retain"]=true;
  doc["entity_category"]="config";
  doc["optimistic"]=false;
  
  doc["state_topic"]=ENABLE_WHEATING_STATE_TOPIC;
  doc["command_topic"]=ENABLE_WHEATING_SET_TOPIC;
  
  DynamicJsonDocument dev=getDeviceBlock();
  doc["dev"]=dev["dev"];
  doc["availability"]=dev["availability"];

  bool published= sendMqttMsg(DISCOVERY_ENABLE_WHEATING_TOPIC.c_str(),doc);
}

void MQTT_DiscoveryMsg_Text_Log(){
  
  DynamicJsonDocument doc(2048);

  doc["name"] = "OpenTherm Log";
  doc["uniq_id"]=MQTT_DEV_UNIQUE_ID+"_OT_LOG";
  doc["icon"]="mdi:radiology-box-outline";
  
  doc["qos"]=0;
  doc["retain"]=true;
  
  doc["state_topic"]=OT_LOG_STATE_TOPIC;
  doc["value_template"]="{{ value_json.text }}";
  
  DynamicJsonDocument dev=getDeviceBlock();
  doc["dev"]=dev["dev"];
  doc["availability"]=dev["availability"];

  bool published= sendMqttMsg(DISCOVERY_OT_LOG_TOPIC.c_str(),doc);

}

void MQTT_DiscoveryMsg_Switch_EnableLog(){

 DynamicJsonDocument doc(2048);

  doc["name"] = "Enable OT Log";
  doc["uniq_id"]=MQTT_DEV_UNIQUE_ID+"_ENABLE_OT_LOG";
  doc["device_class"]="switch";
  doc["icon"]="mdi:radiology-box-outline";
  
  doc["payload_off"]="0";
  doc["payload_on"]="1";
 
  doc["state_off"]="0";
  doc["state_on"]="1";
  
  doc["qos"]=0;
  doc["retain"]=true;
  doc["entity_category"]="config";
  doc["optimistic"]=false;
  
  doc["state_topic"]=ENABLE_OT_LOG_STATE_TOPIC;
  doc["command_topic"]=ENABLE_OT_LOG_SET_TOPIC;
  
  DynamicJsonDocument dev=getDeviceBlock();
  doc["dev"]=dev["dev"];
  doc["availability"]=dev["availability"];

  bool published= sendMqttMsg(DISCOVERY_ENABLE_OT_LOG_TOPIC.c_str(),doc);

}

String convertPayloadToStr(byte* payload, unsigned int length) {
  char s[length + 1];
  s[length] = 0;
  for (int i = 0; i < length; ++i)
    s[i] = payload[i];
  String tempRequestStr(s);
  return tempRequestStr;
}

bool isValidNumber(String str) {
  bool valid = true;
  for (byte i = 0; i < str.length(); i++)
  {
    char ch = str.charAt(i);
    valid &= isDigit(ch) ||
             ch == '+' || ch == '-' || ch == ',' || ch == '.' ||
             ch == '\r' || ch == '\n';
  }
  return valid;
}


void callback(char* topic, byte* payload, unsigned int length) {
  const String topicStr(topic);

  String payloadStr = convertPayloadToStr(payload, length);
  payloadStr.trim();

  if (topicStr == TEMP_SETPOINT_SET_TOPIC) {
    Serial.println("Set target temperature: " + payloadStr);
    float sp1 = payloadStr.toFloat();
    if (isnan(sp1) || !isValidNumber(payloadStr)) {
      Serial.println("Setpoint is not a valid number, ignoring...");
    }
    else {
      Serial.println("setpoint:"+String(sp1));
      publishInitValue5(sp1);
    }
  }
  else if (topicStr == CURRENT_TEMP_SET_TOPIC) {
    float t1 = payloadStr.toFloat();
    if (isnan(t1) || !isValidNumber(payloadStr)) {
      Serial.println("Current temp set is not a valid number, ignoring...");
    }
    else {
      //t = t1;
      //lastTempSet = millis();
      Serial.println("current temp:" +String(t1));
    }
  }
  else if (topicStr == MODE_SET_TOPIC) {
    Serial.println("Set mode: " + payloadStr);
    if (payloadStr == "heat")
      //heatingEnabled = true;
      Serial.println("heating:true");
    else if (payloadStr == "off")
    Serial.println("heating:false");
      //heatingEnabled = false;
    else
      Serial.println("Unknown mode " + payloadStr);
  }
  else if (topicStr == TEMP_DHW_SET_TOPIC) {
    float dhwTarget1 = payloadStr.toFloat();
    if (isnan(dhwTarget1) || !isValidNumber(payloadStr)) {
      Serial.println("DHW target is not a valid number, ignoring...");
    }
    else {
      //dhwTarget = dhwTarget1;
    }
  }
  else if (topicStr == STATE_DHW_SET_TOPIC) {
    if (payloadStr == "on")
      Serial.println("Hotwater:true");
    else if (payloadStr == "off")
        Serial.println("Hotwater:false");
      //enableHotWater = false;
    else
      Serial.println("Unknown domestic hot water state " + payloadStr);
  }
  else if (topicStr == SETPOINT_OVERRIDE_SET_TOPIC) {
    float op_override1 = payloadStr.toFloat();
    if (isnan(op_override1) || !isValidNumber(payloadStr)) {
      Serial.println("Setpoint override is not a valid number, ignoring...");
    }
    else {
      //op_override = op_override1;
      //lastSpSet = millis();
    }
  }
  else if (topicStr == SETPOINT_OVERRIDE_RESET_TOPIC) {
    //lastSpSet = 0;
    Serial.println("Setpoint override reset");
  }
  else {
    Serial.printf("Unknown topic: %s\r\n", topic);
    return;
  }

  //lastUpdate = 0;
}


void publishInitValue1(){

DynamicJsonDocument doc(4096);
String jsonBuffer;

doc["mode"]="heat";

size_t n = serializeJson(doc, jsonBuffer);
Serial.println(jsonBuffer);

bool published=client.publish(MODE_STATE_TOPIC.c_str(), jsonBuffer.c_str(), n);
 
}

void publishInitValue2(){

DynamicJsonDocument doc(4096);
String jsonBuffer;

doc["temperature"]=19.5;

size_t n = serializeJson(doc, jsonBuffer);
Serial.println(jsonBuffer);

bool published=client.publish(CURRENT_TEMP_STATE_TOPIC.c_str(), jsonBuffer.c_str(), n);
 
}

void publishInitValue3(const char * val){

DynamicJsonDocument doc(4096);
String jsonBuffer;

doc["status"]=val;

size_t n = serializeJson(doc, jsonBuffer);
Serial.println(jsonBuffer);

bool published=client.publish(AVAILABILITY_TOPIC.c_str(), jsonBuffer.c_str(), n);
 
}

void publishInitValue4(){

DynamicJsonDocument doc(4096);
String jsonBuffer;

doc["temperature"]=24;

size_t n = serializeJson(doc, jsonBuffer);
Serial.println(jsonBuffer);

bool published=client.publish(TEMP_SETPOINT_STATE_TOPIC.c_str(), jsonBuffer.c_str(), n);
 
}
void publishInitValue5(float tp){

DynamicJsonDocument doc(4096);
String jsonBuffer;

doc["temperature"]=tp;

size_t n = serializeJson(doc, jsonBuffer);
Serial.println(jsonBuffer);

bool published=client.publish(TEMP_SETPOINT_STATE_TOPIC.c_str(), jsonBuffer.c_str(), n);
 
}

void publishInitValue6(){

DynamicJsonDocument doc(4096);
String jsonBuffer;

doc["state"]="ON";

size_t n = serializeJson(doc, jsonBuffer);
Serial.println(jsonBuffer);

bool published=client.publish(ENABLE_WHEATING_SET_TOPIC.c_str(), jsonBuffer.c_str(), n);
 
}